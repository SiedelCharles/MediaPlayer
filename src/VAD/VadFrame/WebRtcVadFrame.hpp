#pragma once
#include "VadFrame.hpp"
#include <fvad.h>
namespace audiotask::vad
{
/// @brief Config of WebRtc config
/// 
struct Config {
    int sample_rate = 16000;     ///< Valid sample rate: {8000, 16000, 32000, 48000}, Internally resampled to 8000 Hz
    int frame_duration_ms = 20;  ///< Valid duration: {10, 20, 30}(ms)
    int mode = 2;                ///< Valid Options: {0, 1, 2, 3}, 0: least aggressive, 3: most aggressive
};
class WebRtcVadFrame : public VadFrame {
private:
    Config _config{};
    Fvad*  _handle{};

    [[nodiscard]] static bool is_config_valid(const Config& config);
    [[nodiscard]] int expected_samples_per_frame() const;
    [[nodiscard]] int expected_bytes_per_frame() const;

public:
    explicit WebRtcVadFrame(const Config& config = {});
    ~WebRtcVadFrame() override;
    virtual size_t frame_size() const noexcept override;
    virtual size_t duration_ms() const noexcept override;
    virtual VadFormat process_frame(std::string_view frame) override;
};   
} // namespace vad
