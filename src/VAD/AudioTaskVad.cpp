#include "AudioTaskVad.hpp"
#include <iostream>

void audiotask::vad::AudioTaskVad::process_data(core::AudioTaskBufferList &&chunk)
{
    auto len = _vadframe->frame_size();
    _bytestream.append(std::move(chunk));
    auto *send_pad = get_pad(core::Direction::Sending);
    auto *receive_pad = get_pad(core::Direction::Receiving);
    if (!receive_pad->is_active()) {
        send_pad->set_active(false);
        while (send_pad->is_active()) {
            send_pad->set_active(false);
        }
        send_pad->push(core::AudioTaskBufferList());
        std::cout << "vad ended" << std::endl;
    }
    while (_bytestream.has(len))
    {
        auto frame_data = _bytestream.peek_copy(len);
        _bytestream.consume(len);
        _timestamp_index += _vadframe->duration_ms();
        auto is_speech = _vadframe->process_frame(frame_data);
        auto state = _mechine.switch_state(is_speech.is_speech);
        switch (state) {
            case VadState::NoSpeeching:
                _buffer.assign(std::move(frame_data)); 
                break;
            case VadState::Beginning:
            case VadState::Speeching:
                _timestamp_duration += _vadframe->duration_ms();
                _buffer.append(frame_data);
                break;
            case VadState::Ending:
                _buffer.append(frame_data);
                _timestamp_duration += 2*_vadframe->duration_ms();
                send_frame(std::move(_buffer));
                _timestamp_duration = 0;
                _buffer.clear(); 
                break;
            default:
                break;
        }
    }

}

void audiotask::vad::AudioTaskVad::send_frame(std::string &&frame)
{
    _timestamp_list.emplace_back(_timestamp_index - _timestamp_duration, _timestamp_index);
    auto back = _timestamp_list.back();
    auto *send_pad = get_pad(core::Direction::Sending);
    auto *receive_pad = get_pad(core::Direction::Receiving);
    if (send_pad) {
        ++_slices;
        if (!receive_pad->is_active()) {
            send_pad->set_active(false);
            while (send_pad->is_active()) {
                send_pad->set_active(false);
            }
        std::cout << "vad ended" << std::endl;
    }
        auto result = send_pad->push(core::AudioTaskBufferList(std::move(frame)));
    }
}
