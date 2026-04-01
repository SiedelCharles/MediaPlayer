#pragma once
#include <atomic>
#include <memory>
#include <vector>
#include <functional>
#include "AudioTaskBuffer.h"
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
    AudioTaskPad(uint32_t id, Direction dir) noexcept : _identifier(id), _direction(dir) {}
    /// @brief Link this pad to a peer pad
    /// @param peerPad Shared pointer to the pad to link with
    /// @return true if link successful, false if pads have same direction or peer is null
    [[nodiscard]] bool link(std::shared_ptr<AudioTaskPad> peer_pad) {
        if (not peer_pad or _direction == peer_pad->_direction) return false;
        _peer_pad = peer_pad;
        peer_pad->_peer_pad = weak_from_this();
        return true;
    }
    /// @brief Unlink this pad from its peer
    /// @details Safely breaks the bidirectional link between pads
    void unlink() noexcept {
        auto peer = _peer_pad.lock();
        if (peer) {
            peer->_peer_pad.reset();
        }
        _peer_pad.reset();
    }
    /// @brief Set the chain function for processing incoming data
    /// @param ChainFunc Function to be called when data is pushed to this pad
    /// @note Typically set on receiving pads to define data processing behavior
    [[nodiscard]] FlowReturn push(AudioTaskBufferList&& buffer) {
        auto peer_pad = _peer_pad.lock();
        if (not peer_pad or not peer_pad->_chain_func) {
            return FlowReturn::Failing;
        }
        return peer_pad->_chain_func(std::move(buffer));
    }
    /// @brief Set the chain function for processing incoming data
    /// @param ChainFunc Function to be called when data is pushed to this pad
    /// @note Typically set on receiving pads to define data processing behavior
    void set_chain_function(ChainFunction func) noexcept {
        _chain_func = std::move(func);
    }
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
/// @brief Base class for audio processing elements
/// @details Elements are processing units that contain pads for linking.
///          Each element can have multiple sending and receiving pads for data flow.
class AudioTaskElement {
protected:
    std::string _name;                                  ///< Human-readable element name
    inline static std::atomic<uint32_t> _succ_pad_id{0};///< brief Global pad ID generator for unique identification
    std::vector<std::shared_ptr<AudioTaskPad>> _pads;   ///< Owned pads collection
public:
    /// @brief Construct a new Audio Task Element
    /// @param name Human-readable name for this element
    explicit AudioTaskElement(const std::string& name = "element") noexcept : _name(name) {}
    virtual ~AudioTaskElement() = default;

    /// @brief Add a new pad to this element
    /// @param direction Data flow direction for the new pad
    /// @return Raw pointer to the created pad (owned by this element)
    /// @note The element retains ownership via shared_ptr
    [[nodiscard]] AudioTaskPad* add_pad(Direction direction) {
        uint32_t id = _succ_pad_id.fetch_add(1, std::memory_order_relaxed);
        auto pad = std::make_shared<AudioTaskPad>(id, direction);
        auto ptr = pad.get();
        _pads.push_back(std::move(pad));
        return ptr;
    }
    /// @brief Find a pad by its unique identifier
    /// @param id Unique pad identifier
    /// @return Pointer to pad if found, nullptr otherwise
    [[nodiscard]] AudioTaskPad* get_pad_by_id(uint32_t id) noexcept {
        for (auto& pad : _pads) {
            if (pad->id() == id) {
                return pad.get();
            }
        }
        return nullptr;
    }
    /// @brief Get the first pad with specified direction
    /// @param direction Direction to search for (Sending or Receiving)
    /// @return Pointer to first matching pad, nullptr if none found
    [[nodiscard]] AudioTaskPad* get_pad(Direction direction) noexcept {
        for (auto& pad : _pads) {
            if (pad->direction() == direction) {
                return pad.get();
            }
        }
        return nullptr;
    }
    /// @brief Link this element to another element
    /// @param other Target element to link to
    /// @return true if link successful, false if pads not found or incompatible
    /// @details Automatically finds first sending pad on this element and first receiving pad on target
    [[nodiscard]] bool link_to(AudioTaskElement* other) {
        if (not other) return false;
        auto src_pad = get_pad(Direction::Sending);
        auto sink_pad = other->get_pad(Direction::Receiving);
        if (not src_pad or not sink_pad) return false;
        // Find shared_ptr for both pads
        auto src_shared = find_shared_ptr(src_pad);
        auto sink_shared = other->find_shared_ptr(sink_pad);
        if (not src_shared or not sink_shared) return false;
        return src_shared->link(sink_shared);
    }
    /// @brief Start the element processing
    /// @return true if started successfully
    /// @note Override in derived classes to implement custom start behavior
    [[nodiscard]] virtual bool start() { return true; }
    /// @brief Stop the element processing
    /// @return true if stopped successfully
    /// @note Override in derived classes to implement custom stop behavior
    [[nodiscard]] virtual bool stop() { return true; }
    /// @brief Get the element's name
    /// @return Const reference to element name
    [[nodiscard]] const std::string& name() const noexcept { return _name; }
protected:
    std::string _name;                                  ///< Human-readable element name
    std::vector<std::shared_ptr<AudioTaskPad>> _pads;   ///< Owned pads collection
    /// @brief Global pad ID generator for unique identification
    static std::atomic<uint32_t> _succ_pad_id;
private:
    /// @brief Find shared_ptr for a raw pad pointer
    /// @param raw Raw pointer to pad
    /// @return Shared pointer if found, nullptr otherwise
    std::shared_ptr<AudioTaskPad> find_shared_ptr(AudioTaskPad* raw) noexcept {
        for (auto& pad : _pads) {
            if (pad.get() == raw) return pad;
        }
        return nullptr;
    }
};