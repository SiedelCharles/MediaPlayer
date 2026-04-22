#pragma once
#include "AudioTaskElement.hpp"
#include "AL/al.h"
#include "AL/alc.h"

namespace audiotask::sink
{
class AudioTaskPlaySink : public core::AudioTaskElement {
public:
    explicit AudioTaskPlaySink(const std::string& name = "PlaySink") noexcept
        : AudioTaskElement(name) {
        auto* pad = add_pad(core::Direction::Receiving);
        pad->set_push_function([this, pad](core::AudioTaskBufferList&& buffer) {
            // 这里可以添加音频播放的逻辑，使用OpenAL进行播放
            // 例如，创建OpenAL缓冲区和源，上传音频数据，并播放
            // 注意处理缓冲区的生命周期和线程安全问题
            return core::FlowReturn::Successful;
        });
    }
};
}