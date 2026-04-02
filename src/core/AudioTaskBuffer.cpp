#include "AudioTaskBuffer.hpp"
#include <stdexcept>
#include <algorithm>

namespace audiotask::core {
StringStorage::StringStorage() : _shared_ref(std::make_shared<std::string>()) {}
StringStorage::StringStorage(std::shared_ptr<std::string> s):_shared_ref(std::move(s)) {}
AVBufferRefStorage::AVBufferRefWrapper::AVBufferRefWrapper(AVBufferRef *ref) : _ref(ref) {
    if (!ref) throw std::invalid_argument("AVBufferRef is null");
}
AVBufferRefStorage::AVBufferRefWrapper::~AVBufferRefWrapper() { if (_ref) av_buffer_unref(&_ref); }
AVBufferRefStorage::AVBufferRefStorage(AVBufferRef *ref) : _shared_ref(std::make_shared<AVBufferRefWrapper>(ref)) {}
AudioTaskBuffer::AudioTaskBuffer() : _storage(StringStorage{std::make_shared<std::string>()}) {}
AudioTaskBuffer::AudioTaskBuffer(std::string &&str) noexcept : _storage(StringStorage(std::make_shared<std::string>(std::move(str)))) {}
AudioTaskBuffer::AudioTaskBuffer(AVBufferRef *ref) : _storage(AVBufferRefStorage(ref)) {}
std::pair<const char*, size_t> AudioTaskBuffer::_storage_view() const {
    return std::visit([](const auto& storage) -> std::pair<const char*, size_t> {
        using Type = std::decay_t<decltype(storage)>;
        if constexpr (std::is_same_v<Type, StringStorage>)
            return {storage._shared_ref->data(), storage._shared_ref->size()};
        else if constexpr (std::is_same_v<Type, AVBufferRefStorage>)
            return {reinterpret_cast<const char*>(storage._shared_ref->_ref->data), storage._shared_ref->_ref->size};
    }, _storage);
}
void AudioTaskBuffer::_reset() noexcept {
    _storage = StringStorage{};
    _offset = 0;
}
std::string_view AudioTaskBuffer::str() const {
    auto [data, size] = _storage_view();
    if (!data || size <= _offset) return {};
    return {data + _offset, size - _offset};
}
uint8_t AudioTaskBuffer::at(const size_t n) const { 
    auto str_view = str();
    if (n >= str_view.size()) throw std::out_of_range("Index out of range");
    return static_cast<uint8_t>(str_view[n]);
}
size_t AudioTaskBuffer::size() const { return str().size(); }
std::string AudioTaskBuffer::copy() const { return std::string(str()); }
void AudioTaskBuffer::remove_prefix(const size_t n) {
    if (n > str().size()) {
        throw std::out_of_range("Buffer::remove_prefix");
    }
    _offset += n;
    auto [_, total] = _storage_view();
    if (_offset >= total) {
        _reset();
    }
}
AudioTaskBufferList::AudioTaskBufferList(AudioTaskBuffer buffer) {
    _buffers.push_back(std::move(buffer));
}
AudioTaskBufferList::AudioTaskBufferList(std::string &&str) {
    _buffers.emplace_back(std::move(str));
}
AudioTaskBufferList::AudioTaskBufferList(AVBufferRef *ref) 
: _buffers{AudioTaskBuffer(ref)} {}
void AudioTaskBufferList::append(const AudioTaskBufferList &other) {
    _buffers.insert(_buffers.end(), other._buffers.begin(), other._buffers.end());
}
void AudioTaskBufferList::append(AudioTaskBufferList &&other) {
    _buffers.insert(_buffers.end(), std::make_move_iterator(other._buffers.begin()), std::make_move_iterator(other._buffers.end()));
    other._buffers.clear();
}
AudioTaskBufferList::operator AudioTaskBuffer() const {
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
void AudioTaskBufferList::remove_prefix(size_t n) {
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
size_t AudioTaskBufferList::size() const {
    size_t total = 0;
    for (const auto &buf : _buffers) {
        total += buf.size();
    }
    return total;
}
std::string AudioTaskBufferList::concatenate() const {
    std::string result;
    result.reserve(size());
    for (const auto &buf : _buffers) {
        result.append(buf);
    }
    return result;
}
std::string AudioTaskBufferList::concatenate(size_t max_len) const {
    std::string result;
    result.reserve(std::min(max_len, size()));
    size_t remaining = max_len;
    for (const auto &buf : _buffers) {
        if (remaining == 0) break;
        size_t to_copy = std::min(remaining, buf.size());
        result.append(buf, 0, to_copy);
        remaining -= to_copy;
    }
    return result;
}
AudioTaskBuffer AudioTaskBufferList::front() const {
    if (_buffers.empty()) {
        throw std::out_of_range("BufferList::front on empty list");
    }
    return _buffers.front();
}
AudioTaskBuffer AudioTaskBufferList::pop_front() {
    if (_buffers.empty()) {
        throw std::out_of_range("BufferList::pop_front on empty list");
    }
    auto buffer = std::move(_buffers.front());
    _buffers.pop_front();
    return buffer;
}
AudioTaskBufferViewList::AudioTaskBufferViewList(const std::string &str) : AudioTaskBufferViewList(std::string_view(str)) {}
AudioTaskBufferViewList::AudioTaskBufferViewList(const char *s) : AudioTaskBufferViewList(std::string_view(s)) {}
AudioTaskBufferViewList::AudioTaskBufferViewList(const AudioTaskBufferList &buffers) {
    for (const auto &x : buffers.buffers()) {
        _views.push_back(x);
    }
}
AudioTaskBufferViewList::AudioTaskBufferViewList(std::string_view str) {_views.push_back(str);}
void AudioTaskBufferViewList::remove_prefix(size_t n) {
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
size_t AudioTaskBufferViewList::size() const {
    size_t ret = 0;
    for (const auto &buf : _views) {
        ret += buf.size();
    }
    return ret;
}
} // namespace audiotask::core