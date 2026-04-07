#pragma once
#include "VadFrame.hpp"
#include "AudioTaskElement.hpp"
#include "AudioTaskByteStream.hpp"
#include "srttimestamp.h"
#include <memory>
namespace audiotask::vad
{
enum class VadState {
    Speeching, NoSpeeching, Beginning, Ending
};
class VadStateMechine {
private:
    VadState _state{VadState::NoSpeeching};
public: 
    const VadState switch_state(bool is_speeching) {
        switch (_state)
        {
        case VadState::NoSpeeching:
            _state = is_speeching ? VadState::Beginning : VadState::NoSpeeching;
            break;
        case VadState::Speeching:
            _state = is_speeching ? VadState::Speeching : VadState::Ending;
            break;
        case VadState::Beginning:
            _state = is_speeching ? VadState::Speeching : VadState::Ending;
            break;
        case VadState::Ending:
            _state = is_speeching ? VadState::Beginning : VadState::NoSpeeching;
            break;
        default:
            break;
        }
        return _state;
    }
};
constexpr uint32_t VadBufferSize = 32000;
constexpr uint32_t VadTimestampSize = 1000;
class AudioTaskVad : public core::AudioTaskElement {
private:
    /// @todo timestamp
    uint64_t _slices{};
    uint64_t _timestamp_index{};
    uint64_t _timestamp_duration{};
    std::vector<TimeStampPair> _timestamp_list{};
    bool _is_running{false};

    std::string _buffer{};
    VadStateMechine _mechine;
    std::unique_ptr<VadFrame> _vadframe;
    core::AudioTaskByteStream _bytestream;
    void process_data(core::AudioTaskBufferList&& chunk);
public: 
    explicit AudioTaskVad(std::unique_ptr<VadFrame> vadframe, const std::string& name = "Vad") noexcept
        : AudioTaskElement(name), _vadframe(std::move(vadframe)) {
            _timestamp_list.reserve(VadTimestampSize);
            _buffer.reserve(VadBufferSize); 
            auto* pad2 = add_pad(core::Direction::Sending);
            auto* pad1 = add_pad(core::Direction::Receiving);
            pad1->set_push_function([this](core::AudioTaskBufferList&& chunk) -> core::FlowReturn{
                this->process_data(std::move(chunk));
                return core::FlowReturn::Successful;
            });
            
            // pad2->set_push_function([this](core::AudioTaskBufferList& chunk) -> core::FlowReturn{
            //     this->process_data(std::move(chunk));
            //     return core::FlowReturn::Successful;
            // });
        }
    void send_frame(std::string&& frame);
};
} // namespace vad
