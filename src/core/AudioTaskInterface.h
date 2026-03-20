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
class AudioTask : public QObject
{
    Q_OBJECT
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
    virtual void cancle() noexcept = 0;
    virtual void decode(const FFmpegFormatConfig& config) = 0;
    virtual void decode(std::function<void(std::span<const uint8_t>)> f_pcmdata) = 0;

    bool is_initialized() const noexcept;
    // virtual void transcode(const QString& input_file, const QString& output_file,
    //                        const QString& target_format, const QVariantMap& options = {}) = 0;
    
    // virtual void merge(const QStringList& inputFiles, const QString& outputFile,
    //                    const QVariantMap& options = {}) = 0;
protected:
    /// @brief subclasses actually initialize ffmpeg resources
    /// @param file_path input file
    /// @return true if initialization succeeds
    virtual bool initialize(const QString& file_path) = 0;
private:
    QMutex              _qmutex_initialized;
    std::atomic<bool>   _atomic_initialized{false};
};

bool AudioTask::init(const QString &file_path) noexcept
{
    /// @brief double-checked locking
    if(_atomic_initialized.load(std::memory_order_acquire)) {
        /// @todo emit error signal
        return false;
    }
    QMutexLocker locker(&_qmutex_initialized);
    if(_atomic_initialized.load(std::memory_order_acquire)) {
        /// @todo emit error signal
        return false;
    }

    bool b_result = initialize(file_path);
    _atomic_initialized.store(b_result, std::memory_order_relaxed);
    return b_result;
}

inline bool AudioTask::is_initialized() const noexcept
{
    return _atomic_initialized.load(std::memory_order_acquire);
}