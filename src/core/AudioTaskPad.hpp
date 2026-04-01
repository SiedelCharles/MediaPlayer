#pragma once
#include <atomic>
#include <memory>
#include <vector>
#include <functional>
#include "AudioTaskBuffer.hpp"
namespace audiotask::core {
/// @brief Direction: direction of data flow in Pad
/// @brief FlowReturn: status code returned by Pad operations
enum class Direction {Sending, Receiving};
enum class FlowReturn {Ended, Failing, Flushing, Successful};
/// @brief Audio processing pad for linking audio task elements
/// @details Pads are connection points between audio processing elements.
///          Sending pads push data, Receiving pads receive data via chain functions.
class AudioTaskPad : public std::enable_shared_from_this<AudioTaskPad> {
    /// @name callback functions
    /// @{
    using ChainFunction = std::function<FlowReturn(AudioTaskBufferList&&)>;
    /// @todo Add event and query callback functions
    /// @}
private:
    uint32_t _identifier;                   ///< Unique identifier for this pad
    Direction _direction;                   ///< Data flow direction
    std::weak_ptr<AudioTaskPad> _peer_pad;  ///< Weak reference to linked peer pad
    ChainFunction _chain_func;              ///< Callback for processing incoming data
public:
    /// @brief Construct a new Audio Task Pad
    /// @param id Unique identifier for this pad
    /// @param direction Data flow direction (Source or Sink)
    AudioTaskPad(uint32_t id, Direction dir) noexcept;
    /// @brief Link this pad to a peer pad
    /// @param peerPad Shared pointer to the pad to link with
    /// @return true if link successful, false if pads have same direction or peer is null
    [[nodiscard]] bool link(std::shared_ptr<AudioTaskPad> peer_pad);
    /// @brief Unlink this pad from its peer
    /// @details Safely breaks the bidirectional link between pads
    void unlink() noexcept;
    /// @brief Set the chain function for processing incoming data
    /// @param ChainFunc Function to be called when data is pushed to this pad
    /// @note Typically set on receiving pads to define data processing behavior
    [[nodiscard]] FlowReturn push(AudioTaskBufferList&& buffer);
    /// @brief Set the chain function for processing incoming data
    /// @param ChainFunc Function to be called when data is pushed to this pad
    /// @note Typically set on receiving pads to define data processing behavior
    void set_chain_function(ChainFunction func) noexcept;
    /// @brief Get the pad's unique identifier
    /// @return Const reference to pad ID
    [[nodiscard]] const uint32_t& id() const noexcept { return _identifier; }
    /// @brief Get the pad's data flow direction
    /// @return Direction enum value
    [[nodiscard]] Direction direction() const noexcept { return _direction; }
    /// @brief Get the linked peer pad
    /// @return Shared pointer to peer pad, or nullptr if not linked
    [[nodiscard]] std::shared_ptr<AudioTaskPad> peer_pad() const noexcept { return _peer_pad.lock(); }
};
} // namespace audiotask::core
