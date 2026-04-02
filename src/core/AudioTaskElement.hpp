#pragma once
#include "AudioTaskPad.hpp"
#include "AudioTaskBuffer.hpp"
#include <atomic>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>
namespace audiotask::core {
/// @brief Base class for audio processing elements
/// @details Elements are processing units that contain pads for linking.
///          Each element can have multiple sending and receiving pads for data flow.
class AudioTaskElement {
protected:
    std::string _name;                                  ///< Human-readable element name
    std::vector<std::shared_ptr<AudioTaskPad>> _pads;   ///< Owned pads collection
    /// @brief Global pad ID generator for unique identification
    inline static std::atomic<uint32_t> _succ_pad_id{0};
private:
    /// @brief Find shared_ptr for a raw pad pointer
    /// @param raw Raw pointer to pad
    /// @return Shared pointer if found, nullptr otherwise
    std::shared_ptr<AudioTaskPad> find_shared_ptr(AudioTaskPad* raw) noexcept;
public:
    /// @brief Construct a new Audio Task Element
    /// @param name Human-readable name for this element
    explicit AudioTaskElement(const std::string& name = "element") noexcept;
    virtual ~AudioTaskElement() = default;

    /// @brief Add a new pad to this element
    /// @param direction Data flow direction for the new pad
    /// @return Raw pointer to the created pad (owned by this element)
    /// @note The element retains ownership via shared_ptr
    [[nodiscard]] AudioTaskPad* add_pad(Direction direction);
    /// @brief Find a pad by its unique identifier
    /// @param id Unique pad identifier
    /// @return Pointer to pad if found, nullptr otherwise
    [[nodiscard]] AudioTaskPad* get_pad_by_id(uint32_t id) noexcept;
    /// @brief Get the first pad with specified direction
    /// @param direction Direction to search for (Sending or Receiving)
    /// @return Pointer to first matching pad, nullptr if none found
    [[nodiscard]] AudioTaskPad* get_pad(Direction direction) noexcept;
    /// @brief Link this element to another element
    /// @param other Target element to link to
    /// @return true if link successful, false if pads not found or incompatible
    /// @details Automatically finds first sending pad on this element and first receiving pad on target
    [[nodiscard]] bool link_to(AudioTaskElement* other);
    /// @brief Start the element processing
    /// @return true if started successfully
    /// @note Override in derived classes to implement custom start behavior
    [[nodiscard]] virtual bool start();
    /// @brief Stop the element processing
    /// @return true if stopped successfully
    /// @note Override in derived classes to implement custom stop behavior
    [[nodiscard]] virtual bool stop();
    /// @brief Get the element's name
    /// @return Const reference to element name
    [[nodiscard]] const std::string& name() const noexcept { return _name; }
};
} // namespace audiotask::core