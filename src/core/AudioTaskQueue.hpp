#include "AudioTaskElement.hpp"
#include "AudioTaskBufferStream.hpp"
#include <stdexcept>
#include <iostream>
namespace audiotask::core
{
class AudioTaskQueue : public AudioTaskElement {
private:
    AudioTaskBufferStream _bufferstream;
public:
    explicit AudioTaskQueue(uint64_t capacity, const std::string& name = "BufferQueue") : _bufferstream(capacity), AudioTaskElement(name) {
        auto *receive_pad = add_pad(Direction::Receiving);
        receive_pad->set_push_function([this](AudioTaskBufferList&& frame) ->FlowReturn {
            while (frame.count() > 0) {
                _bufferstream.write(std::move(frame.pop_front()));
            }
            return FlowReturn::Successful;
        });
        auto *sending_pad = add_pad(Direction::Sending);
        sending_pad->set_pull_function([this](AudioTaskBufferList& frame) ->FlowReturn {
            frame = AudioTaskBufferList(_bufferstream.try_pop());
            if (frame.size() > 0) {
                return FlowReturn::Successful;
            }
            auto* pad = get_pad(Direction::Receiving);
            if (!pad->is_active()) {
                return FlowReturn::Ended;
            }
            return FlowReturn::Failing;
        });
    }
};
} // namespace audiotask::core
