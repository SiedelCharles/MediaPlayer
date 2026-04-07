#include "AudioTaskPad.hpp"
#include <iostream>
namespace audiotask::core {
AudioTaskPad::AudioTaskPad(uint32_t id, Direction dir) noexcept : _identifier(id), _direction(dir) {}
bool AudioTaskPad::link(std::shared_ptr<AudioTaskPad> peer_pad) {
    if (not peer_pad or _direction == peer_pad->_direction) return false;
    _peer_pad = peer_pad;
    peer_pad->_peer_pad = weak_from_this();
    return true;
}
void AudioTaskPad::unlink() noexcept {
    auto peer = _peer_pad.lock();
    if (peer) {
        peer->_peer_pad.reset();
    }
    _peer_pad.reset();
}
FlowReturn AudioTaskPad::push(AudioTaskBufferList&& buffer) {
    auto peer_pad = _peer_pad.lock();
    if (not peer_pad or not peer_pad->_push_func) {
        return FlowReturn::Failing;
    }
    if (!is_active()) {
        peer_pad->set_active(false);
    }
    return peer_pad->_push_func(std::move(buffer));
}
FlowReturn AudioTaskPad::pull(AudioTaskBufferList &buffer)
{
    auto peer_pad = _peer_pad.lock();
    if (not peer_pad or not peer_pad->_pull_func) {
        return FlowReturn::Failing;
    }
    if (!is_active()) {
        peer_pad->set_active(false);
    }
    return peer_pad->_pull_func(buffer);
}
void AudioTaskPad::set_push_function(PushFunction func) noexcept
{
    _push_func = std::move(func);
}
void AudioTaskPad::set_pull_function(PullFunction func) noexcept
{
    _pull_func = std::move(func);
}
} // namespace audiotask::core
