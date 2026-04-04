#pragma once
#include <mutex>
#include <string>
#include <cstdint>
#include <algorithm>
#include <condition_variable>
#include "AudioTaskBuffer.hpp"
namespace audiotask::core {
/// @brief Thread-safe byte stream for audio data buffering
/// 
/// This class provides a bounded, thread-safe queue for AudioTaskBuffer objects.
/// It supports blocking push/pop operations with capacity management based on total bytes.
/// Similar to GStreamer's internal queue mechanism.
/// 
/// @note This is a pure data structure with no Qt dependencies for maximum reusability
class AudioTaskBufferStream {
private:
    /// @name Buffer related
    /// @{
    uint64_t _capacity{0};        ///< Maximum capacity in bytes
    uint64_t _current_size{0};    ///< Current size in bytes
    AudioTaskBufferList _buffers; ///< Buffer queue
    /// @}
    /// @name Control related
    /// @{
    bool _error{false};     ///< Error flag
    bool _aborted{false};   ///< Abort flag
    bool _finished{false};  ///< Input ended flag
    bool _flushing{false};  ///< Flushing flag
    mutable std::mutex _mutex;
    std::condition_variable _condition_read;    ///< Signaled when queue is not empty
    std::condition_variable _condition_write;   ///< Signaled when queue is not full
    /// @}
public:
    /// @brief Construct a byte stream with specified capacity
    /// @param capacity Maximum capacity in bytes
    explicit AudioTaskBufferStream(uint64_t capacity) noexcept : _capacity(capacity) {}
    /// @brief Destructor
    ~AudioTaskBufferStream() noexcept = default;
    /// @brief Explicitly delete copy constructor, copy assignment, move constructor, and move assignment
    AudioTaskBufferStream(const AudioTaskBufferStream&) = delete;
    AudioTaskBufferStream& operator=(const AudioTaskBufferStream&) = delete;
    AudioTaskBufferStream(AudioTaskBufferStream&&) = delete;
    AudioTaskBufferStream& operator=(AudioTaskBufferStream&&) = delete;
    
    /// @todo Supplement read/write interfaces with additional data types

    /// @name "Input" interface for the writer
    /// @{
    /// @brief Write a buffer into the stream (blocking)
    /// @param buffer Buffer to write
    /// @return Number of bytes accepted, 0 if aborted
    /// @note Blocks if queue is full until space is available
    [[nodiscard]] size_t write(AudioTaskBuffer buffer);
    /// @brief Try to write a buffer without blocking
    /// @param buffer Buffer to write
    /// @return Number of bytes accepted, 0 if full or aborted
    [[nodiscard]] size_t try_write(AudioTaskBuffer buffer); 
    /// @brief Signal that the byte stream has reached its ending
    void end_input() noexcept;
    /// @brief Get the number of additional bytes that the stream has space for
    /// @return Remaining capacity in bytes
    [[nodiscard]] inline size_t remaining_capacity() const noexcept {
        std::lock_guard<std::mutex> lock(_mutex);
        return _capacity > _current_size ? _capacity - _current_size : 0;
    }
    /// @brief Indicate that the stream suffered an error
    inline void set_error() noexcept { 
        std::lock_guard<std::mutex> lock(_mutex);
        _error = true; 
    } 
    /// @}
    
    /// @name "Output" interface for the reader
    /// @{
    /// @details this is moved to bytestream
    // /// @brief Peek at next "len" bytes of the stream without removing them
    // /// @param len Number of bytes to peek
    // /// @return String containing peeked data
    // [[nodiscard]] std::string peek_output(size_t len) const;
    // /// @brief Remove bytes from the buffer
    // /// @param len Number of bytes to remove
    // void pop_output(size_t len);
    // /// @brief Read (i.e., peek and then pop) the next "len" bytes of the stream
    // /// @param len Number of bytes to read
    // /// @return String containing read data
    // [[nodiscard]] inline std::string read(size_t len) {
    //     auto ret = peek_output(len);
    //     pop_output(ret.size());
    //     return ret;
    // }
    /// @brief Pop a buffer from the stream (blocking)
    /// @return Buffer pointer, or nullptr if stream ended or aborted
    /// @note Blocks if queue is empty until data is available or stream ends
    [[nodiscard]] AudioTaskBuffer pop();
    /// @brief Try to pop a buffer without blocking
    /// @return Buffer pointer, or nullptr if empty, ended, or aborted
    [[nodiscard]] AudioTaskBuffer try_pop();
    /// @brief Check if the stream input has ended
    /// @return true if input ended, false otherwise
    [[nodiscard]] inline bool input_ended() const noexcept {
        std::lock_guard<std::mutex> lock(_mutex);
        return _finished;
    }
    /// @brief Check if the stream has suffered an error
    /// @return true if error occurred, false otherwise
    [[nodiscard]] inline bool error() const noexcept { 
        std::lock_guard<std::mutex> lock(_mutex);
        return _error; 
    }
    /// @brief Get the maximum amount that can currently be read from the stream
    /// @return Buffer size in bytes
    [[nodiscard]] inline size_t buffer_size() const noexcept {
        std::lock_guard<std::mutex> lock(_mutex);
        return _current_size;
    }
    /// @brief Check if the buffer is empty
    /// @return true if empty, false otherwise
    [[nodiscard]] inline bool buffer_empty() const noexcept {
        std::lock_guard<std::mutex> lock(_mutex);
        return _buffers.empty();
    }
    /// @brief Check if the output has reached the ending
    /// @return true if buffer is empty and input ended, false otherwise
    [[nodiscard]] inline bool eof() const noexcept {
        std::lock_guard<std::mutex> lock(_mutex);
        return _finished && _buffers.empty();
    }
    /// @}
    
    /// @name Additional operations
    /// @{
    /// @brief Abort the stream
    /// @note Wakes up all waiting threads and causes operations to return immediately
    void abort() noexcept;
    /// @brief Clear all buffers in the stream
    void clear() noexcept;
    /// @brief Begin flushing operation
    /// @note Clears all buffers and sets flushing state
    void flush_begin() noexcept;
    /// @brief End flushing operation
    /// @note Resets flushing state and allows normal operation
    void flush_end() noexcept;
    /// @brief Check if the stream is flushing
    /// @return true if flushing, false otherwise
    [[nodiscard]] inline bool is_flushing() const noexcept {
        std::lock_guard<std::mutex> lock(_mutex);
        return _flushing;
    }
    /// @brief Get maximum capacity in bytes
    /// @return Maximum capacity in bytes
    [[nodiscard]] inline uint64_t capacity() const noexcept { return _capacity; }
    /// @brief Get number of buffers in the queue
    /// @return Number of buffers
    [[nodiscard]] inline size_t count() const noexcept {
        std::lock_guard<std::mutex> lock(_mutex);
        return _buffers.size();
    }
    /// @brief Check if the stream is full
    /// @return true if full, false otherwise
    [[nodiscard]] inline bool full() const noexcept {
        std::lock_guard<std::mutex> lock(_mutex);
        return _current_size >= _capacity;
    }
    /// @brief Check if the stream is aborted
    /// @return true if aborted, false otherwise
    [[nodiscard]] inline bool is_aborted() const noexcept {
        std::lock_guard<std::mutex> lock(_mutex);
        return _aborted;
    }
    /// @}
};
}