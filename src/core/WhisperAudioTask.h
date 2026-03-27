#pragma once
#include "AudioTaskInterface.h"

extern "C" {
    #include "whisper.h"
}

/// @brief Transcription mode for whisper inference.
enum class WhisperTranscriptionMode {
    Offline,   ///< Load entire audio once and call whisper_full (batch mode).
    Streaming  ///< Process audio in chunks, maintain context via whisper_full_with_state (streaming mode).
};

class WhisperTaskProcesser {
public:
    ~WhisperTaskProcesser() = default;
    virtual void transcribe(const struct whisper_full_params& full_params, WhisperTranscriptionMode mode, const QString& file_path = QString{}) = 0;
};

/// @brief This class is used for whisper transcription task
/// @details Accept processed data only; audio preprocessing class is already implemented by another class.
/// @todo It is recommended to separate parameter configuration into an independent class
class WhisperAudioTask : public AudioTaskBase
                       , public WhisperTaskProcesser
{
    Q_OBJECT
signals:
    void text_transcribed(const QString& text);
public:
    WhisperAudioTask(QObject* parent = nullptr) noexcept : AudioTaskBase(AudioTaskBufferRole::Input, parent) {};
    /// @brief transcription using full_whisper, offline transcription (audio length within tens of minutes, can be loaded at once)
    /// @param file_path path of output file, now unused
    /// @param full_params whisper_full_params, here are 53 parameters in it
    /// @details Some parameters in full_params (such as print_realtime, print_progress) may not be applicable in streaming mode.
    /// @param mode choose offline, streaming or more type to transcribe
    virtual void transcribe(const struct whisper_full_params& full_params, WhisperTranscriptionMode mode, [[maybe_unused]]const QString& file_path = QString{}) override;
protected:
    /// @brief this is default initialize, it does not throw exceptions because the whisper.cpp API is C-style.
    /// @param file_path the path of model
    /// @return 'true' if whisper_context initialized successfully
    /// @todo Supplement the settings for whisper_context_params
    virtual bool initialize(const QString& file_path) noexcept override;
private:
    /// @brief Convert pcm data to 32-bit floating point, Require 16-bit signed integer
    QVector<float> convert_to_float32(const QByteArray &pcm_data); ///< return QVector<float>
    bool convert_to_float32(QVector<float>& buffer, const QByteArray &pcm_data); ///< using buffer, not implemented yet

    /// @brief The concrete implementation of the transcription function
    void transcribe_offline(const QString& file_path, const struct whisper_full_params& full_params);
    void transcribe_streaming(const QString& file_path, const struct whisper_full_params& full_params);
    
    struct whisper_context* _whisper_context{nullptr};
};