#pragma once
#include "AudioTaskSource.hpp"
#include "AudioTaskElement.hpp"
#include <mutex>
namespace audiotask::source {/// @brief Format:16-bit signed, 32-bit signed, 32-bit float, 64-bit float
enum class SampleFormat {Undefined, S16, S32, F32, F64};
/// @brief Config for source
/// @todo To be extended into a class similar to GStreamer's Caps.
struct SourceFormat{
    int channels{0};
    int sample_rate{0};
    double duration{0.0};
    SampleFormat format{SampleFormat::Undefined};
};
/// @brief This class handles audio decoding
class FileSource : public AudioTaskSource {
protected:
    mutable std::mutex _state_mutex;
    /// @brief file releated
    std::string _file_path;
    std::atomic<bool> _initialized{false};
    /// @brief source format, output format
    SourceFormat _source_format{};
    SourceFormat _output_format{};
public:
    explicit FileSource(const std::string& name = "FileSource") noexcept 
        : AudioTaskSource(name) {}
    FileSource(const FileSource&) = delete;
    FileSource& operator=(const FileSource&) = delete;
    FileSource(FileSource&&) = delete;
    FileSource& operator=(FileSource&&) = delete;
    /// @brief Initialize the file source
    /// @return true if initialization succeeds, false if already initialized or failed
    [[nodiscard]] virtual bool init(const std::string& file_path) noexcept = 0;
    /// @brief  Seek to a specified time point
    /// @param time_sec seconds, absolute time
    /// @return true if seek succeeds
    [[nodiscard]] virtual bool seek(double time_sec) noexcept = 0;
    /// @brief  close file
    [[nodiscard]] virtual bool close() noexcept = 0;
    
    [[nodiscard]] bool is_initialized() const noexcept {
        return _initialized.load();
    }
    [[nodiscard]] std::string file_path() const noexcept {
        std::lock_guard<std::mutex> lock(_state_mutex);
        return _file_path;
    }
    [[nodiscard]] SourceFormat source_format() const noexcept {
        std::lock_guard<std::mutex> lock(_state_mutex);
        return _source_format;
    }
    [[nodiscard]] SourceFormat output_format() const noexcept {
        std::lock_guard<std::mutex> lock(_state_mutex);
        return _output_format;
    }
    void set_output_format(const SourceFormat& fmt) noexcept {
        std::lock_guard<std::mutex> lock(_state_mutex);
        if (!_running.load()) {
            _output_format = fmt;
        }
    }
    [[nodiscard]] bool start() override {
        if (!_initialized.load()) return false;
        return AudioTaskSource::start();
    }
};
}