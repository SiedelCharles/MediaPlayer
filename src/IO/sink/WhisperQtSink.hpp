// AudioTaskAsrSink.hpp
#pragma once
#include "AudioTaskElement.hpp"
#include "QtCore/qobject.h"
#include "QtCore/qstring.h"
#include <mutex>
#include <vector>
#include "AL/al.h"
namespace audiotask::sink
{
class AudioTaskAsrSink : public QObject, public core::AudioTaskElement {
    Q_OBJECT
signals:
    void text_recognized(const size_t vad_count, const QString& text);
    void text_done();
public:
    explicit AudioTaskAsrSink(const std::string& name = "AsrSink") noexcept
        : QObject(nullptr), AudioTaskElement(name) {
        auto* pad = add_pad(core::Direction::Receiving);
        pad->set_push_function([this, pad](core::AudioTaskBufferList&& buffer) {
            if (buffer.count() > 1) {
                auto front = buffer.pop_front();
                auto text = buffer.concatenate();
                text_recognized(std::stoi(front.copy()), QString::fromStdString(text));
            }
            if (!pad->is_active()) {
                emit text_done();
            }
            return core::FlowReturn::Successful;
        });
    }
};

} // namespace audiotask::asr
