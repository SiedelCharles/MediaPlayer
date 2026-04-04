#pragma once
#include "AudioTaskElement.hpp"
#include "VadProcessor.hpp"
#include <memory>
namespace audiotask::vad
{
class StateMechine {
    
};
class AudioTaskVad : public core::AudioTaskElement {
private:
    std::unique_ptr<VadProcessor> _processor{nullptr};
public: 
    /// @todo State
    explicit AudioTaskVad(std::unique_ptr<VadFrame> vadframe, const std::string& name = "Vad") noexcept
        :  AudioTaskElement(name) {
            _processor = std::make_unique<VadProcessor>(std::move(vadframe));
            auto* pad = add_pad(core::Direction::Receiving);
            pad->set_chain_function([this](core::AudioTaskBufferList&& chunk) -> core::FlowReturn {
            /// @todo Add logic to handle the return value after calling process
            _processor->process(std::move(chunk));
            return core::FlowReturn::Successful;
        });
        }
};
} // namespace vad
