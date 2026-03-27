#include "srttimestamp.h"
#include "WhisperAudioTask.h"

#include <iostream>

void WhisperAudioTask::transcribe(const struct whisper_full_params& full_params, WhisperTranscriptionMode mode, [[maybe_unused]]const QString& file_path) {
    if (auto b_result = is_initialized()
        ; !b_result) {
        /// @todo emit error
        
        return ;
    }
    switch(mode) {
        case WhisperTranscriptionMode::Offline:
            transcribe_offline(file_path, full_params);
            break;
        case WhisperTranscriptionMode::Streaming:
            transcribe_streaming(file_path, full_params);
            break;
    }
    /// @todo this part should be encapsulated into a cleanup function.
    whisper_free(_whisper_context);
    _whisper_context = nullptr;
    emit message_finished();
    stop();
}

bool WhisperAudioTask::initialize(const QString &file_path) noexcept
{
    QByteArray path = file_path.toUtf8();
    struct whisper_context_params params = whisper_context_default_params();
    _whisper_context = whisper_init_from_file_with_params(path.constData(), params);
    return _whisper_context != nullptr;
}

/// @todo Consider alignment and big/little endian
QVector<float> WhisperAudioTask::convert_to_float32(const QByteArray &pcm_data)
{
    /// @todo optional: Use SIMD acceleration
    QVector<float> pcm_f32;
    pcm_f32.reserve(pcm_data.size() >> 1);

    const int16_t *pcm_16 = reinterpret_cast<const int16_t*>(pcm_data.constData());
    int sample_count = pcm_data.size() / sizeof(int16_t);

    for (int i = 0; i < sample_count; ++i) {
        pcm_f32.push_back(static_cast<float>(pcm_16[i]) / 32768.0f);
    }
    /// @brief use RVC
    return pcm_f32;
}

/// @todo complete this version of convert_to_float32
bool WhisperAudioTask::convert_to_float32(QVector<float> &buffer, const QByteArray &pcm_data)
{
    return false;
}

void WhisperAudioTask::transcribe_offline(const QString &file_path, const whisper_full_params &full_params)
{
    QVector<float> accumulated_data;
    while (true) {
        {
            QMutexLocker locker(&_qmutex_buffer);
            while (_data_buffer.isEmpty() && !_atomic_stop.load(std::memory_order_acquire)
             && !_atomic_cancel.load(std::memory_order_acquire) && !_atomic_eof.load(std::memory_order_acquire)) {
                _qcondition_wait.wait(&_qmutex_buffer);
            }
            /// @todo
            if (_atomic_cancel.load(std::memory_order_acquire)) {
                _data_buffer.clear();
                return;
            }
            if (_data_buffer.isEmpty()) {
                break;
            }
        }
        /// @todo to optimize here
        QList<QByteArray> current_batch = take_data();
        for (const QByteArray &data : qAsConst(current_batch)) {
            auto data_float = convert_to_float32(data);
            accumulated_data.append(data_float.begin(), data_float.end());
        }
    }

    if (auto i_result = whisper_full(_whisper_context, full_params, accumulated_data.data(), accumulated_data.size())
        ; i_result != 0) {
            /// @todo emit error signal;
            return ;
        }
    int segments = whisper_full_n_segments(_whisper_context);
    QString transcribed_text;
    for (int i = 0; i < segments; ++i) {
        const char *text = whisper_full_get_segment_text(_whisper_context, i);
        int64_t t0 = whisper_full_get_segment_t0(_whisper_context, i);
        int64_t t1 = whisper_full_get_segment_t1(_whisper_context, i);
        /// @todo related to timestamp part in utils
        std::string timestamp = convert_to_srt_timestamp(t0*10, t1*10);
        transcribed_text += QString::fromStdString(timestamp) +  " " + QString::fromUtf8(text) + "\n" ;
    }
    /// @todo emit data
    emit text_transcribed(transcribed_text);
    return ;
}

void WhisperAudioTask::transcribe_streaming(const QString &file_path, const whisper_full_params &full_params)
{
    return ;
}
