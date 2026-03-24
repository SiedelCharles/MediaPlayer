#include "AudioTaskInterface.h"
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

void AudioTask::cancle() noexcept
{
    _atomic_cancel.store(true, std::memory_order_acquire);
}