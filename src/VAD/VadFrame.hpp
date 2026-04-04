#pragma once
#include "AudioTaskBuffer.hpp"

namespace audiotask::vad
{
/// @brief return type of Vad
struct VadFormat {
    bool is_speech{false};
    double confidence{0.0};
};
class VadFrame {
public:
    virtual ~VadFrame() = default;
    virtual size_t frame_size() const noexcept = 0;
    virtual VadFormat process_frame(std::string_view frame) = 0;
};
} // namespace vad


