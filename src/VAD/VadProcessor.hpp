#pragma once
#include "AudioTaskBuffer.hpp"
#include "AudioTaskByteStream.hpp"

#include "VadFrame.hpp"
namespace audiotask::vad
{
class VadProcessor {
private:
    std::unique_ptr<VadFrame> _vadframe;
    core::AudioTaskByteStream _bytestream;
public:
    explicit VadProcessor(std::unique_ptr<VadFrame> vadframe) noexcept
        : _vadframe(std::move(vadframe)) {}
    void process(core::AudioTaskBufferList&& buffer);
};
} // namespace vad
