#pragma once

#include "QtCore/qmutex.h"
#include "QtCore/qobject.h"
#include "QtCore/qstringlist.h"
#include "QtCore/qvariantmap.h"

/// @brief It is unnecessary since Qt has already adopted precompiled headers
#include <span>
#include <atomic>
#include <cstdint>
#include <functional>

constexpr size_t AV_ERROR_MAX_BUFFER_SIZE = 64;

class FFmpegFormatConfig;

/// @brief this class is for audio decode,transcode, and merge
class AudioTask : public QObject
{
    Q_OBJECT
signals:    
    void message_ffmpeg(const QString& msg);
    void message_finished();
public:
    explicit AudioTask(QObject* parent = nullptr) noexcept : QObject(parent) {};
    ~AudioTask() override = default;

    /// @brief Explicitly delete copy constructor, copy assignment, move constructor, and move assignment
    AudioTask(const AudioTask&) = delete;
    AudioTask& operator=(const AudioTask&) = delete;
    AudioTask(AudioTask&&) = delete;
    AudioTask& operator=(AudioTask&&) = delete;

    /// @brief Initialize ffmpeg resources, this function is thread-safe.
    /// @param file_path Input file
    /// @return true if initialization succeeds
    [[nodiscard]] bool init(const QString& file_path) noexcept;
    /// @brief 
    virtual void cancle() noexcept;
    virtual void merge(const QStringList& input_file, const QString& output_file) = 0;
    virtual void decode(const FFmpegFormatConfig& config) = 0;
    virtual void decode(std::function<void(std::span<const uint8_t>)> f_pcmdata) = 0;
    virtual void transcode(const QStringList& input_file, const QString& output_file) = 0;

    bool is_initialized() const noexcept;
protected:
    /// @brief subclasses actually initialize ffmpeg resources
    /// @param file_path input file
    /// @return true if initialization succeeds
    virtual bool initialize(const QString& file_path) = 0;
    std::atomic<bool>   _atomic_cancle{false};
private:
    QMutex              _qmutex_initialized;
    std::atomic<bool>   _atomic_initialized{false};
};

inline bool AudioTask::is_initialized() const noexcept
{
    return _atomic_initialized.load(std::memory_order_acquire);
}