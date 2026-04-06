#pragma once
#include "AudioTaskElement.hpp"
#include <thread>
#include <atomic>
namespace audiotask::asr
{
constexpr size_t MAX_TRANSCRIBED_TEXT_SIZE = 1000;
class AudioTaskAsr : public core::AudioTaskElement {
protected:
    std::thread _thread;                ///< Worker thread for data generation
    std::atomic<bool> _running{false};  ///< Thread running state flag
    virtual void run() = 0;
    std::vector<std::pair<size_t, std::string>> transcribed_text{};
public:
explicit AudioTaskAsr(const std::string& name = "Asr") noexcept
        : AudioTaskElement(name) {
        add_pad(core::Direction::Receiving);
        transcribed_text.reserve(MAX_TRANSCRIBED_TEXT_SIZE);
    }
    ~AudioTaskAsr() override {
        stop();
    }
        /// @brief Start the source element
    /// @details Creates worker thread and begins data generation
    /// @return true if started successfully, false if already running
    [[nodiscard]] bool start() override {
        if (_running.exchange(true)) return false;
        if (_thread.joinable()) _thread.join();
        _thread = std::thread([this]() { run(); });
        return true;
    }
    /// @brief Stop the source element
    /// @details Signals thread to stop and waits for completion
    /// @return true if stopped successfully, false if not running
    [[nodiscard]] bool stop() override {
        const bool is_running = _running.exchange(false);
        if (_thread.joinable()) _thread.join();
        return is_running;
    }
    [[nodiscard]] const std::vector<std::pair<size_t, std::string>>& get_text() const {
        /// @todo keep thread-safe;
        return transcribed_text;
    }
};
} // namespace audio::asr
