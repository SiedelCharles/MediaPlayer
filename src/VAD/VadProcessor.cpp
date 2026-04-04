#include "VadProcessor.hpp"

namespace audiotask::vad
{
void audiotask::vad::VadProcessor::process(core::AudioTaskBufferList &&buffer)
{
    auto len = _vadframe->frame_size();
    _bytestream.append(std::move(buffer));
    while (_bytestream.has(len))
    {
        auto res = _bytestream.peek_copy(len);
        _bytestream.consume(len);
        auto is_speech = _vadframe->process_frame(res);
    }
}
} // namespace audiotask::vad

