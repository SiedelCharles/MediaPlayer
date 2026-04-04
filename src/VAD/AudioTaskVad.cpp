#include "AudioTaskVad.hpp"

void audiotask::vad::AudioTaskVad::process_data(core::AudioTaskBufferList &&chunk)
{
    auto len = _vadframe->frame_size();
    _bytestream.append(std::move(chunk));
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
    auto *send_pad = get_pad(core::Direction::Sending);
    if (send_pad) {
        auto result = send_pad->push(core::AudioTaskBufferList(std::move(frame)));
    }
}
