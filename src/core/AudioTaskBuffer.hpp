#pragma once
/// @brief std library
#include <deque>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <variant>
#include <string_view>
/// @todo I commented out these headers to prevent pollution (e.g., std::min and std::max)
// /// @brief iovec if Linux, WSABUF for Windows
// #if defined(_WIN32)
// #include <WinSock2.h>
// #else
// #include <sys/uio.h>
// #endif
/// @brief ffmpeg library, to use AVBufferRef
extern "C" {
#include <libavutil/buffer.h>
}
namespace audiotask::core {
/// @brief declares and implements
/// @name Storage type
/// @{
struct StringStorage { 
    std::shared_ptr<std::string> _shared_ref;
    StringStorage();
    explicit StringStorage(std::shared_ptr<std::string> s);
};
struct AVBufferRefStorage {
    struct AVBufferRefWrapper {
        AVBufferRef *_ref;
        explicit AVBufferRefWrapper(AVBufferRef *ref);
        ~AVBufferRefWrapper();
        AVBufferRefWrapper(const AVBufferRefWrapper&) = delete;
        AVBufferRefWrapper& operator=(const AVBufferRefWrapper&) = delete;
    };
    std::shared_ptr<AVBufferRefWrapper> _shared_ref;
    explicit AVBufferRefStorage(AVBufferRef *ref);
};
using BufferStorage = std::variant<StringStorage, AVBufferRefStorage>;
/// @}
class AudioTaskBuffer {
private:
    uint64_t _offset{};
    BufferStorage _storage;
    [[nodiscard]] std::pair<const char*, size_t> _storage_view() const;
    void _reset() noexcept;
public:
    AudioTaskBuffer();
    explicit AudioTaskBuffer(std::string &&str) noexcept;
    explicit AudioTaskBuffer(AVBufferRef *ref) noexcept;
    /// @name Expose contents as a std::string_view
    [[nodiscard]] std::string_view str() const;
    [[nodiscard]] operator std::string_view() const { return str(); }
    /// @}
    /// @brief Get character at location `n`
    [[nodiscard]] uint8_t at(const size_t n) const;
    /// @brief Is buffers empty
    [[nodiscard]] size_t empty() const { return size() == 0; }
    /// @brief Size of the string
    [[nodiscard]] size_t size() const;
    /// @brief Make a copy to a new std::string
    [[nodiscard]] std::string copy() const;
    /// @brief Discard the first `n` bytes of the string (does not require a copy or move)
    /// @note Doesn't free any memory until the whole string has been discarded in all copies of the Buffer.
    void remove_prefix(const size_t n);
};
class AudioTaskBufferList {
private:
    std::deque<AudioTaskBuffer> _buffers;
public:
    /// @name Constructors
    ///@{
    AudioTaskBufferList() = default;
    /// @brief Construct from a Buffer
    explicit AudioTaskBufferList(AudioTaskBuffer buffer);
    /// @brief Construct by taking ownership of a std::string
    explicit AudioTaskBufferList(std::string &&str);
    /// @brief Construct by AVBufferRef*
    explicit AudioTaskBufferList(AVBufferRef *ref);
    ///@}
    /// @brief Access the underlying queue of Buffers
    const std::deque<AudioTaskBuffer> &buffers() const { return _buffers; }
    /// @brief Append a BufferList
    void append(const AudioTaskBufferList &other);
    void append(AudioTaskBufferList &&other);
    /// @brief Transform to a Buffer
    /// @note Throws an exception unless BufferList is contiguous
    operator AudioTaskBuffer() const;
    /// @brief Discard the first `n` bytes of the string (does not require a copy or move)
    void remove_prefix(size_t n);
    /// @brief Is buffers empty
    [[nodiscard]] bool empty() const { return size() == 0;}
    /// @brief Size of the string
    [[nodiscard]] size_t size() const;
    /// @brief Make a copy to a new std::string
    [[nodiscard]] std::string concatenate() const;
    [[nodiscard]] std::string concatenate(size_t max_len) const;
    /// @brief Get the first buffer
    [[nodiscard]] AudioTaskBuffer front() const;
    /// @brief Pop the first buffer and remove it
    [[nodiscard]] AudioTaskBuffer pop_front();
};
/// @brief A non-owning temporary view (similar to std::string_view) of a discontiguous string
class AudioTaskBufferViewList {
private:
    std::deque<std::string_view> _views{};
public:
    /// @name Constructors
    ///@{
    /// @brief Construct from a std::string
    AudioTaskBufferViewList(const std::string &str);
    /// @brief Construct from a C string (must be NULL-terminated)
    AudioTaskBufferViewList(const char *s);
    /// @brief Construct from a BufferList
    explicit AudioTaskBufferViewList(const AudioTaskBufferList &buffers);
    /// @brief Construct from a std::string_view
    AudioTaskBufferViewList(std::string_view str);
    ///@}
    /// @brief Discard the first `n` bytes of the string (does not require a copy or move)
    void remove_prefix(size_t n);
    /// @brief Size of the string
    [[nodiscard]] size_t size() const;
    /// @brief Convert to a vector of `iovec`(for Linux) structures, and `WSABUF`(for Windows)
    // std::vector<iovec> as_iovecs() const;
};
} // namespace audiotask::core