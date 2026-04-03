#pragma once

#include <string>
#include <cstdint>
#include <optional>
#include <string_view>

#include "AudioTaskBuffer.hpp"

namespace audiotask::core {

/// @brief Non-thread-safe byte stream adapter for audio data
///
/// This class provides a logical contiguous byte stream view over a sequence
/// of AudioTaskBuffer objects. It is intended for intra-element use cases such as:
/// - assembling fixed-size PCM frames for VAD
/// - accumulating stream fragments from upstream
/// - reading/peeking/consuming arbitrary byte counts
///
/// Unlike AudioTaskBufferStream, this class is not a blocking queue and does not
/// perform any thread synchronization. It is a pure adapter over AudioTaskBufferList.
///
/// The internal storage may be physically discontiguous while remaining logically
/// contiguous to the caller.
///
/// @note This class does not guarantee that peeked data is stored contiguously
/// unless peek_contiguous() succeeds.
class AudioTaskByteStream {
private:
    /// @name Buffer related
    /// @{
    uint64_t _current_size{0};    ///< Current size in bytes
    AudioTaskBufferList _buffers; ///< Underlying logically contiguous buffer list
    /// @}

public:
    /// @name Constructors and assignment
    /// @{
    AudioTaskByteStream() = default;
    ~AudioTaskByteStream() = default;

    AudioTaskByteStream(const AudioTaskByteStream&) = default;
    AudioTaskByteStream& operator=(const AudioTaskByteStream&) = default;
    AudioTaskByteStream(AudioTaskByteStream&&) noexcept = default;
    AudioTaskByteStream& operator=(AudioTaskByteStream&&) noexcept = default;
    /// @}

    /// @name Append interface
    /// @{
    /// @brief Append a buffer list to the end of the stream
    void append(const AudioTaskBufferList& buffers);

    /// @brief Append a buffer list to the end of the stream
    void append(AudioTaskBufferList&& buffers);
    /// @}

    /// @name Read state query
    /// @{
    /// @brief Get current stream size in bytes
    /// @return Current readable size in bytes
    [[nodiscard]] inline size_t size() const noexcept {
        return static_cast<size_t>(_current_size);
    }

    /// @brief Check whether the stream is empty
    /// @return true if empty, false otherwise
    [[nodiscard]] inline bool empty() const noexcept {
        return _current_size == 0;
    }

    /// @brief Check whether at least len bytes are available
    /// @param len Required number of bytes
    /// @return true if at least len bytes are available
    [[nodiscard]] inline bool has(size_t len) const noexcept {
        return _current_size >= len;
    }
    /// @}

    /// @name Peek interface
    /// @{
    /// @brief Peek at the next len bytes without consuming them
    /// @param len Number of bytes to peek
    /// @return Buffer view list representing up to len bytes from the front of the stream
    /// @note Returned data may be physically discontiguous
    [[nodiscard]] AudioTaskBufferViewList peek(size_t len) const;

    /// @brief Peek at exactly the next len bytes without consuming them
    /// @param len Number of bytes to peek
    /// @return Buffer view list representing exactly len bytes from the front of the stream;
    ///         std::nullopt if fewer than len bytes are available
    /// @note Returned data may be physically discontiguous
    [[nodiscard]] std::optional<AudioTaskBufferViewList> peek_exact(size_t len) const;

    /// @brief Peek at the next len bytes as a contiguous copied string
    /// @param len Number of bytes to peek
    /// @return String containing up to len bytes copied from the front of the stream
    [[nodiscard]] std::string peek_copy(size_t len) const;

    /// @name Consume/read interface
    /// @{
    /// @brief Remove len bytes from the front of the stream
    /// @param len Number of bytes to consume
    /// @note If len exceeds current size, all available data is consumed
    void consume(size_t len);

    /// @brief Read the next len bytes as a contiguous copied string
    /// @param len Number of bytes to read
    /// @return String containing up to len bytes copied from the front of the stream
    /// @note Equivalent to peek_copy(len) followed by consume()
    [[nodiscard]] std::string read_copy(size_t len);

    /// @brief Take the next len bytes from the stream as a buffer list
    /// @param len Number of bytes to take
    /// @return Buffer list representing up to len bytes from the front of the stream
    /// @note This operation consumes the returned bytes from the stream
    [[nodiscard]] AudioTaskBufferList take(size_t len);
    /// @}

    /// @name Additional operations
    /// @{
    /// @brief Clear the entire stream
    void clear() noexcept;
    /// @}
};

}  // namespace audiotask::core
