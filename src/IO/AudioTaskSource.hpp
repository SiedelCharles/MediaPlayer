#pragma once
#include "AudioTaskElement.hpp"
#include <thread>
#include <atomic>
namespace audiotask::source {
/// @brief Base class for audio source elements that generate data
/// @details Sources run in a separate thread and push data downstream via sending pad.
///          Subclasses implement run() to define data generation logic.
class AudioTaskSource : public core::AudioTaskElement {
protected:
    std::thread _thread;                ///< Worker thread for data generation
    std::atomic<bool> _running{false};  ///< Thread running state flag
    /// @brief Data generation loop, implemented by subclass
    /// @details Called in worker thread. Should check _running flag and exit when false.
    ///          Use get_pad(Direction::Sending) to access output pad.
    virtual void run() = 0;
public:
    /// @brief Construct a new source element
    /// @param name Human-readable element name
    explicit AudioTaskSource(const std::string& name = "Source") noexcept
        : AudioTaskElement(name) {
        add_pad(core::Direction::Sending);
    }
    ~AudioTaskSource() override {
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
        if (!_running.exchange(false)) return false;
        if (_thread.joinable()) _thread.join();
        return true;
    }
};
} // namespace audiotask::source
