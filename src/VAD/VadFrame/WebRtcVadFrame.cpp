#include "WebRtcVadFrame.hpp"
#include <stdexcept>
namespace audiotask::vad
{
bool WebRtcVadFrame::is_config_valid(const Config& config)
{
    bool is_sample_rate_valid = config.sample_rate == 8000 
                             || config.sample_rate == 16000
                             || config.sample_rate == 32000
                             || config.sample_rate == 48000;
    bool is_duration_ms_valid = config.frame_duration_ms == 10
                             || config.frame_duration_ms == 20
                             || config.frame_duration_ms == 30;
    bool is_mode_valid = config.mode >= 0 && config.mode <= 3;
    return is_sample_rate_valid && is_duration_ms_valid && is_mode_valid;
}

int WebRtcVadFrame::expected_samples_per_frame() const
{
    return (_config.sample_rate * _config.frame_duration_ms) / 1000;
}

int WebRtcVadFrame::expected_bytes_per_frame() const
{
    return expected_samples_per_frame() * static_cast<int>(sizeof(int16_t));
}

WebRtcVadFrame::WebRtcVadFrame(const Config &config) : _config(config)
{
    if (!is_config_valid(_config)) {
        throw std::invalid_argument("invalid config");
    }
    _handle = fvad_new();
    if (_handle == nullptr) {
        throw std::runtime_error("fvad_new failed");
    }

    if (auto i_result = fvad_set_sample_rate(_handle, _config.sample_rate)
        ; i_result < 0) {
        throw std::runtime_error("fvad_set_sample_rate failed");
    }
    if (auto i_result = fvad_set_mode(_handle, _config.mode)
        ; i_result < 0) {
        throw std::runtime_error("fvad_set_mode failed");
    }
}

vad::WebRtcVadFrame::~WebRtcVadFrame()
{
    if (_handle) {
        fvad_free(_handle);
        /// @todo delete follow sentence
        _handle = nullptr;
    }
}

size_t vad::WebRtcVadFrame::frame_size() const noexcept
{
    return static_cast<size_t>(expected_bytes_per_frame());
}

size_t vad::WebRtcVadFrame::duration_ms() const noexcept
{
    return _config.frame_duration_ms;
}

VadFormat vad::WebRtcVadFrame::process_frame(std::string_view frame)
{
if (_handle == nullptr) {
        throw std::runtime_error("WebRtcVadFrame: VAD handle is null");
    }

    const int expected_bytes = expected_bytes_per_frame();
    if (static_cast<int>(frame.size()) != expected_bytes) {
        throw std::invalid_argument("WebRtcVadFrame: invalid frame size");
    }

    const auto* audio_frame = reinterpret_cast<const int16_t*>(frame.data());
    const int samples = expected_samples_per_frame();

    const int result = fvad_process(
        _handle,
        audio_frame,
        samples
    );

    if (result < 0) {
        throw std::runtime_error("WebRtcVadFrame: WebRtcVad_Process failed");
    }

    VadFormat output;
    output.is_speech = (result == 1);
    output.confidence = output.is_speech ? 1.0 : 0.0;
    return output;
}
} // namespace vad