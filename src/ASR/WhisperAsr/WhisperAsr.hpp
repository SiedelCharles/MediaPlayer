#pragma once
#include "AudioTaskAsr.hpp"

extern "C" {
    #include "whisper.h"
}

#include <mutex>
namespace audiotask::asr
{
/// @brief Transcription mode for whisper inference.
enum class WhisperTranscriptionMode {
    Offline,   ///< Load entire audio once and call whisper_full (batch mode).
    Streaming  ///< Process audio in chunks, maintain context via whisper_full_with_state (streaming mode).
};

// class WhisperTaskProcesser {
// public:
//     ~WhisperTaskProcesser() = default;
//     virtual void transcribe(const struct whisper_full_params& full_params, WhisperTranscriptionMode mode, const QString& file_path = QString{}) = 0;
// };

/// @brief This class is used for whisper transcription task
/// @details Accept processed data only; audio preprocessing class is already implemented by another class.
/// @todo It is recommended to separate parameter configuration into an independent class
class WhisperAsr : public AudioTaskAsr
{
private:
    mutable std::mutex _state_mutex;
    /// @brief model releated
    std::string _model_path;
    std::atomic<bool> _initialized{false};
    struct whisper_context* _whisper_context{nullptr};
    struct whisper_full_params _whisper_full_params;

public:
    explicit WhisperAsr(const std::string& name = "WhisperAsr") noexcept 
        : AudioTaskAsr(name) {}
    [[nodiscard]] bool init(const std::string& model_path, const struct whisper_full_params& full_params);
    /// @brief this is default initialize, it does not throw exceptions because the whisper.cpp API is C-style.
    /// @return 'true' if whisper_context initialized successfully
    /// @todo Supplement the settings for whisper_context_params
    [[nodiscard]] bool is_initialized() const noexcept {
        return _initialized.load();
    }
    virtual void run() override;
private:
    /// @brief Convert pcm data to 32-bit floating point, Require 16-bit signed integer
    std::vector<float> convert_to_float32(const core::AudioTaskBufferList& pcm_data);

    /// @brief The concrete implementation of the transcription function
    // void transcribe_offline(const QString& file_path, const struct whisper_full_params& full_params);
    // void transcribe_streaming(const QString& file_path, const struct whisper_full_params& full_params);
    
    
};
} // namespace audiotask::asr