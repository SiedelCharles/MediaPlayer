#include "AudioTaskElement.hpp"
namespace audiotask::core {
AudioTaskElement::AudioTaskElement(const std::string& name) noexcept : _name(name) {}
AudioTaskPad* AudioTaskElement::add_pad(Direction direction) {
    uint32_t id = _succ_pad_id.fetch_add(1, std::memory_order_relaxed);
    auto pad = std::make_shared<AudioTaskPad>(id, direction);
    auto ptr = pad.get();
    _pads.push_back(std::move(pad));
    return ptr;
}
AudioTaskPad* AudioTaskElement::get_pad_by_id(uint32_t id) noexcept {
    for (auto& pad : _pads) {
        if (pad->id() == id) {
            return pad.get();
        }
    }
    return nullptr;
}
AudioTaskPad* AudioTaskElement::get_pad(Direction direction) noexcept {
    for (auto& pad : _pads) {
        if (pad->direction() == direction) {
            return pad.get();
        }
    }
    return nullptr;
}
bool AudioTaskElement::link_to(AudioTaskElement* other) {
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
bool AudioTaskElement::start() { return false; }
bool AudioTaskElement::stop() { return true; }
std::shared_ptr<AudioTaskPad> AudioTaskElement::find_shared_ptr(AudioTaskPad* raw) noexcept {
    for (auto& pad : _pads) {
        if (pad.get() == raw) return pad;
    }
    return nullptr;
}
} // namespace audiotask::core