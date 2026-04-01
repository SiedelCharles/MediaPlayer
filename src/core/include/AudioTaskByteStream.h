#pragma once
#include <mutex>
#include <cstdint>
#include <condition_variable>
/// @brief Audio tasks related
#include "AudioTaskBuffer.h"
/// @brief Thread-safe byte stream for audio data buffering
/// 
/// This class provides a bounded, thread-safe queue for AudioTaskBuffer objects.
/// It supports blocking push/pop operations with capacity management based on total bytes.
/// Similar to GStreamer's internal queue mechanism.
/// 
/// @note This is a pure data structure with no Qt dependencies for maximum reusability
class AudioTaskByteStream {
private:
    /// @name Buffer related
    /// @{
    uint64_t _capacity{0};        ///< Maximum capacity in bytes
    uint64_t _current_size{0};    ///< Current size in bytes
    AudioTaskBufferList _buffers; ///< Buffer queue
    /// @}
    /// @name Control related
    /// @{
    bool _eos{false};      ///< End of stream flag
    bool _aborted{false};  ///< Abort flag
    mutable std::mutex _mutex;
    std::condition_variable _condition_not_full;  ///< Signaled when queue is not full
    std::condition_variable _condition_not_empty; ///< Signaled when queue is not empty
    /// @}
public:
    /// @brief Construct a byte stream with specified capacity
    /// @param capacity Maximum capacity in bytes
    explicit AudioTaskByteStream(uint64_t capacity) noexcept : _capacity(capacity) {}
    /// @brief Destructor
    ~AudioTaskByteStream() noexcept = default;
    /// @brief Explicitly delete copy constructor, copy assignment, move constructor, and move assignment
    AudioTaskByteStream(const AudioTaskByteStream&) = delete;
    AudioTaskByteStream& operator=(const AudioTaskByteStream&) = delete;
    AudioTaskByteStream(AudioTaskByteStream&&) = delete;
    AudioTaskByteStream& operator=(AudioTaskByteStream&&) = delete;
    
    /// @brief Push a buffer into the stream (blocking)
    /// @param buffer Buffer to push
    /// @return true if successful, false if aborted
    /// @note Blocks if queue is full until space is available
    [[nodiscard]] bool push(std::shared_ptr<AudioTaskBuffer> buffer);
    
    /// @brief Pop a buffer from the stream (blocking)
    /// @return Buffer pointer, or nullptr if stream ended or aborted
    /// @note Blocks if queue is empty until data is available or stream ends
    [[nodiscard]] std::shared_ptr<AudioTaskBuffer> pop();
    
    /// @brief Try to push a buffer without blocking
    /// @param buffer Buffer to push
    /// @return true if successful, false if full or aborted
    [[nodiscard]] bool try_push(std::shared_ptr<AudioTaskBuffer> buffer);
    
    /// @brief Try to pop a buffer without blocking
    /// @return Buffer pointer, or nullptr if empty, ended, or aborted
    [[nodiscard]] std::shared_ptr<AudioTaskBuffer> try_pop();
    
    /// @brief Mark the stream as ended (no more data will be pushed)
    /// @note Wakes up all waiting consumers
    void set_eos() noexcept;
    
    /// @brief Abort the stream
    /// @note Wakes up all waiting threads and causes push/pop to return immediately
    void abort() noexcept;
    
    /// @brief Clear all buffers in the stream
    void clear() noexcept;
    
    /// @brief Get current size in bytes
    /// @return Current size in bytes
    [[nodiscard]] uint64_t size() const noexcept;
    
    /// @brief Get maximum capacity in bytes
    /// @return Maximum capacity in bytes
    [[nodiscard]] uint64_t capacity() const noexcept { return _capacity; }
    
    /// @brief Get number of buffers in the queue
    /// @return Number of buffers
    [[nodiscard]] size_t count() const noexcept;
    
    /// @brief Check if the stream is full
    /// @return true if full, false otherwise
    [[nodiscard]] bool full() const noexcept;
    
    /// @brief Check if the stream is empty
    /// @return true if empty, false otherwise
    [[nodiscard]] bool empty() const noexcept;
    
    /// @brief Check if the stream has ended
    /// @return true if EOS is set, false otherwise
    [[nodiscard]] bool is_eos() const noexcept;
    
    /// @brief Check if the stream is aborted
    /// @return true if aborted, false otherwise
    [[nodiscard]] bool is_aborted() const noexcept;
};

// /// @brief Qt library
// #include "QtCore/qmutex.h"
// #include "QtCore/qobject.h"
// #include "QtCore/qwaitcondition.h"
// #include "QtCore/qsharedpointer.h"
// /// @brief stdandard library, it is unnecessary since Qt has already adopted precompiled headers
// #include <span>
// #include <atomic>
// #include <cstdint>
// #include <utility>
// 
// /// @todo move the constexpr to other file
// constexpr size_t AV_ERROR_MAX_BUFFER_SIZE = 64;

// /// @brief Bytes stream 
// class AudioTaskByteStream {
// private:
//     /// @name buffer releated
//     /// @{
//     uint64_t _capacity{0};
//     AudioTaskBufferList _buffers{};
//     /// @}
//     /// @name control releated
//     /// @{
//     bool _ended{false};
//     bool _aborted{false};
//     mutable std::mutex  _mutex;
//     std::condition_variable _condition_full{false};
//     std::condition_variable _condition_empty{true};
//     /// @}
// public:
//     explicit AudioTaskByteStream(const size_t capacity) : _capacity(capacity) {}
//     /// @brief Explicitly delete copy constructor, copy assignment, move constructor, and move assignment
//     AudioTaskByteStream(const AudioTaskByteStream&) = delete;
//     AudioTaskByteStream& operator=(const AudioTaskByteStream&) = delete;
//     AudioTaskByteStream(AudioTaskByteStream&&) = delete;
//     AudioTaskByteStream& operator=(AudioTaskByteStream&&) = delete;


// };

// /// @brief This class handles audio tasks, binds to a specific file type upon initialization and provides a data buffer
// class AudioTaskBase : public QObject
//                     , public AudioTaskByteStream
// {
//     Q_OBJECT
// signals:
//     void message_finished();
// public:
//     explicit AudioTaskBase(QObject* parent = nullptr) noexcept : QObject(parent) {}; // AudioTaskBufferType type = AudioTaskBufferType::Default, _role_buffer(type)
//     ~AudioTaskBase() override = default;

//     /// @brief Explicitly delete copy constructor, copy assignment, move constructor, and move assignment
//     AudioTaskBase(const AudioTaskBase&) = delete;
//     AudioTaskBase& operator=(const AudioTaskBase&) = delete;
//     AudioTaskBase(AudioTaskBase&&) = delete;
//     AudioTaskBase& operator=(AudioTaskBase&&) = delete;

//     /// @brief Initialize ffmpeg resources, this function is thread-safe.
//     /// @param file_path input file
//     /// @return 'true' if initialization succeeds
//     [[nodiscard]] bool init(const QString& file_path) noexcept;
//     bool is_initialized() const noexcept;

//     /// @brief Stop receiving or produce data to the buffer
//     void eof() noexcept;
//     bool is_eof() noexcept;
//     /// @brief get buffer mode  
//     AudioTaskBufferType mode() noexcept;
//     /// @brief Graceful stop, exit after processing all pending data
//     virtual void stop() noexcept;
//     /// @brief Stop immediately, exit with discarding pending data
//     virtual void cancel() noexcept;
//     /// @brief get data from the buffer
//     QList<QByteArray> take_data() noexcept;
//     /// @brief append data to the buffer
//     void append_data(const QByteArray& data) noexcept;
//     void append_data(QSharedPointer<QByteArray> pdata) noexcept;
//     /// @brief clean uo the data in the buffer
//     void cleanup_data() noexcept;
// protected:
//     /// @brief subclasses actually initialize ffmpeg resources
//     /// @param file_path input file
//     /// @return true if initialization succeeds
//     virtual bool initialize(const QString& file_path) noexcept = 0;

//     /// @brief path of file, can be an audio or a model 
//     QString             _file_path;
//     /// @brief control-related params
//     std::atomic<bool>   _atomic_eof{false};
//     std::atomic<bool>   _atomic_stop{false};
//     std::atomic<bool>   _atomic_cancel{false};
//     AudioTaskBufferType _role_buffer;
//     QWaitCondition      _qcondition_wait;
//     /// @brief buffer-related params
//     QMutex              _qmutex_buffer;
//     QList<QByteArray>   _data_buffer{};
// private:
//     QMutex              _qmutex_initialized;
//     std::atomic<bool>   _atomic_initialized{false};
// };

// inline bool AudioTaskBase::init(const QString &file_path) noexcept
// {
//     /// @brief double-checked locking
//     if (_atomic_initialized.load(std::memory_order_acquire)) {
//         /// @todo emit error signal
//         return false;
//     }
//     QMutexLocker locker(&_qmutex_initialized);
//     if (_atomic_initialized.load(std::memory_order_acquire)) {
//         /// @todo emit error signal
//         return false;
//     }

//     bool b_result = initialize(file_path);
//     if (b_result) {
//         _atomic_initialized.store(true, std::memory_order_relaxed);
//         _file_path = file_path;
//     }
//     return b_result;
// }

// inline bool AudioTaskBase::is_initialized() const noexcept
// {
//     return _atomic_initialized.load(std::memory_order_acquire);
// }

// inline void AudioTaskBase::eof() noexcept
// {
//     _atomic_eof.store(true, std::memory_order_release);
//     _qcondition_wait.wakeAll();
// }

// inline bool AudioTaskBase::is_eof() noexcept {
//     return _atomic_eof.load(std::memory_order_acquire);
// }

// inline void AudioTaskBase::stop() noexcept
// {
//     _atomic_stop.store(true, std::memory_order_release);
//     _qcondition_wait.wakeAll();
// }

// inline AudioTaskBufferType AudioTaskBase::mode() noexcept
// {
//     return _role_buffer;
// }

// inline void AudioTaskBase::cancel() noexcept
// {
//     _atomic_cancel.store(true, std::memory_order_release);
//     _qcondition_wait.wakeAll();
// }

// inline QList<QByteArray> AudioTaskBase::take_data() noexcept {
//     QMutexLocker locker(&_qmutex_buffer);
//     return std::exchange(_data_buffer, {});
// }

// inline void AudioTaskBase::append_data(const QByteArray &data) noexcept
// {
//     QMutexLocker locker(&_qmutex_buffer);
//     _data_buffer.append(data);
//     _qcondition_wait.wakeOne();
// }

// inline void AudioTaskBase::append_data(QSharedPointer<QByteArray> pdata) noexcept
// {
//     QMutexLocker locker(&_qmutex_buffer);
//     _data_buffer.append(*pdata);
//     _qcondition_wait.wakeOne();
// }

// inline void AudioTaskBase::cleanup_data() noexcept
// {
//     QMutexLocker locker(&_qmutex_buffer);
//     _data_buffer.clear();
// }