#pragma once
/// @brief std library
#include <deque>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <numeric>
#include <variant>
#include <stdexcept>
#include <string_view>
/// @brief iovec if Linux, WSABUF for Windows
#if defined(_WIN32)
#include <WinSock2.h>
#else
#include <sys/uio.h>
#endif
/// @brief ffmpeg library, to use AVBufferRef
extern "C" {
#include <libavutil/buffer.h>
}
/// @brief declares and implements
/// @name Storage type
/// @{
struct StringStorage { 
    std::shared_ptr<std::string> _shared_ref;
    StringStorage() : _shared_ref(std::make_shared<std::string>()) {}
    explicit StringStorage(std::shared_ptr<std::string> s):_shared_ref(std::move(s)) {}
};
struct AVBufferRefStorage {
    struct AVBufferRefWrapper {
        AVBufferRef *_ref;
        AVBufferRefWrapper(AVBufferRef *ref) {
            if (!ref) throw std::invalid_argument("AVBufferRef is null");
            _ref = av_buffer_ref(ref);
            if (!_ref) throw std::invalid_argument("av_buffer_ref failed");
        }
        ~AVBufferRefWrapper() { if (_ref) av_buffer_unref(&_ref); }
        AVBufferRefWrapper(const AVBufferRefWrapper&) = delete;
        AVBufferRefWrapper& operator=(const AVBufferRefWrapper&) = delete;
    };
    std::shared_ptr<AVBufferRefWrapper> _shared_ref;
    explicit AVBufferRefStorage(AVBufferRef *ref) : _shared_ref(std::make_shared<AVBufferRefWrapper>(ref)) {}
};
using BufferStorage = std::variant<StringStorage, AVBufferRefStorage>;
/// @}
class AudioTaskBuffer {
private:
    uint64_t _offset{};
    BufferStorage _storage;
    [[nodiscard]] std::pair<const char*, size_t> _storage_view() const {
        return std::visit([](const auto& storage) -> std::pair<const char*, size_t> {
            using Type = std::decay_t<decltype(storage)>;
            if constexpr (std::is_same_v<Type, StringStorage>)
                return {storage._shared_ref->data(), storage._shared_ref->size()};
            else if constexpr (std::is_same_v<Type, AVBufferRefStorage>)
                return {reinterpret_cast<const char*>(storage._shared_ref->_ref->data), storage._shared_ref->_ref->size};
        }, _storage);
    }
    void _reset() noexcept {
        _storage = StringStorage{};
        _offset = 0;
    }
public:
    AudioTaskBuffer() : _storage(StringStorage{std::make_shared<std::string>()}) {}
    explicit AudioTaskBuffer(std::string &&str) noexcept : _storage(StringStorage(std::make_shared<std::string>(std::move(str)))) {}
    explicit AudioTaskBuffer(AVBufferRef *ref) noexcept : _storage(AVBufferRefStorage(ref)) {}
    /// @name Expose contents as a std::string_view
    [[nodiscard]] std::string_view str() const {
        auto [data, size] = _storage_view();
        if (!data || size <= _offset) return {};
        return {data + _offset, size - _offset};
    }
    [[nodiscard]] operator std::string_view() const { return str(); }
    /// @}
    /// @brief Get character at location `n`
    [[nodiscard]] uint8_t at(const size_t n) const { 
        auto str_view = str();
        if (n >= str_view.size()) throw std::out_of_range("Index out of range");
        return static_cast<uint8_t>(str_view[n]);
    }
    /// @brief Size of the string
    [[nodiscard]] size_t size() const { return str().size(); }
    /// @brief Make a copy to a new std::string
    [[nodiscard]] std::string copy() const { return std::string(str()); }
    /// @brief Discard the first `n` bytes of the string (does not require a copy or move)
    /// @note Doesn't free any memory until the whole string has been discarded in all copies of the Buffer.
    void remove_prefix(const size_t n) {
        if (n > str().size()) {
            throw std::out_of_range("Buffer::remove_prefix");
        }
        _offset += n;
        auto [_, total] = _storage_view();
        if (_offset >= total) {
            _reset();
        }
    }
};
class AudioTaskBufferList {
private:
    std::deque<AudioTaskBuffer> _buffers;
public:
    /// @name Constructors
    ///@{
    AudioTaskBufferList() = default;
    /// @brief Construct from a Buffer
    explicit AudioTaskBufferList(AudioTaskBuffer buffer) {
        _buffers.push_back(std::move(buffer));
    }
    /// @brief Construct by taking ownership of a std::string
    explicit AudioTaskBufferList(std::string &&str) {
        _buffers.emplace_back(std::move(str));
    }
    /// @brief Construct by AVBufferRef*
    explicit AudioTaskBufferList(AVBufferRef *ref) 
    : _buffers{AudioTaskBuffer(ref)} {}
    ///@}
    /// @brief Access the underlying queue of Buffers
    const std::deque<AudioTaskBuffer> &buffers() const { return _buffers; }
    /// @brief Append a BufferList
    void append(const AudioTaskBufferList &other) {
        _buffers.insert(_buffers.end(), other._buffers.begin(), other._buffers.end());
    }
    void append(AudioTaskBufferList &&other) {
        _buffers.insert(_buffers.end(), std::make_move_iterator(other._buffers.begin()), std::make_move_iterator(other._buffers.end()));
        /// @todo clear other
    }
    /// @brief Transform to a Buffer
    /// @note Throws an exception unless BufferList is contiguous
    operator AudioTaskBuffer() const {
        switch (_buffers.size()) {
        case 0:
            return {};
        case 1:
            return _buffers[0];
        default: {
            throw std::runtime_error(
                "BufferList: please use concatenate() to combine a multi-Buffer BufferList into one Buffer");
            }
        }
    }
    /// @brief Discard the first `n` bytes of the string (does not require a copy or move)
    void remove_prefix(size_t n) {
        if (n > size()) {
            throw std::out_of_range("BufferList::remove_prefix");
        }
        
        while (n > 0) {
            auto& front = _buffers.front();
            size_t front_size = front.size();
            
            if (n < front_size) {
                front.remove_prefix(n);
                return;
            }
            
            n -= front_size;
            _buffers.pop_front();
        }
    }
    /// @brief Size of the string
    [[nodiscard]] size_t size() const {
        size_t total = 0;
        for (const auto &buf : _buffers) {
            total += buf.size();
        }
        return total;
    };
    /// @brief Make a copy to a new std::string
    [[nodiscard]] std::string concatenate() const {
        std::string result;
        result.reserve(size());
        for (const auto &buf : _buffers) {
            result.append(buf);
        }
        return result;
    }
};
/// @brief A non-owning temporary view (similar to std::string_view) of a discontiguous string
class AudioTaskBufferViewList {
private:
    std::deque<std::string_view> _views{};
public:
    /// @name Constructors
    ///@{
    /// @brief Construct from a std::string
    AudioTaskBufferViewList(const std::string &str) : AudioTaskBufferViewList(std::string_view(str)) {}
    /// @brief Construct from a C string (must be NULL-terminated)
    AudioTaskBufferViewList(const char *s) : AudioTaskBufferViewList(std::string_view(s)) {}
    /// @brief Construct from a BufferList
    explicit AudioTaskBufferViewList(const AudioTaskBufferList &buffers) {
        for (const auto &x : buffers.buffers()) {
            _views.push_back(x);
        }
    }
    /// @brief Construct from a std::string_view
    AudioTaskBufferViewList(std::string_view str) { _views.push_back({const_cast<char *>(str.data()), str.size()}); }
    ///@}
    /// @brief Discard the first `n` bytes of the string (does not require a copy or move)
    void remove_prefix(size_t n) {
        while (n > 0) {
            if (_views.empty()) {
                throw std::out_of_range("BufferListView::remove_prefix");
            }
            if (n < _views.front().size()) {
                _views.front().remove_prefix(n);
                n = 0;
            } else {
                n -= _views.front().size();
                _views.pop_front();
            }
        }
    }
    /// @brief Size of the string
    [[nodiscard]] size_t size() const {
        size_t ret = 0;
        for (const auto &buf : _views) {
            ret += buf.size();
        }
        return ret;
    }
    /// @brief Convert to a vector of `iovec`(for Linux) structures, and `WSABUF`(for Windows)
    // std::vector<iovec> as_iovecs() const;
};