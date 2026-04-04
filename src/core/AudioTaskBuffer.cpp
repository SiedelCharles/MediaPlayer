#include "AudioTaskBuffer.hpp"

#include <iterator>
#include <algorithm>
#include <stdexcept>
#include <type_traits>

namespace audiotask::core {

/// @brief StringStorage
StringStorage::StringStorage() : _shared_ref(std::make_shared<std::string>()) {}

StringStorage::StringStorage(std::shared_ptr<std::string> s) : _shared_ref(std::move(s)) {}

/// @brief AVBufferRefStorage Wrapper
AVBufferRefStorage::AVBufferRefWrapper::AVBufferRefWrapper(AVBufferRef* ref, uint64_t valid_size) : _ref(ref), _valid_size(valid_size) {
    if (_ref == nullptr) {
        throw std::invalid_argument("AVBufferRef is null");
    }
}

AVBufferRefStorage::AVBufferRefWrapper::~AVBufferRefWrapper() {
    if (_ref != nullptr) {
        av_buffer_unref(&_ref);
    }
}

AVBufferRefStorage::AVBufferRefStorage(AVBufferRef* ref, uint64_t valid_size)
    : _shared_ref(std::make_shared<AVBufferRefWrapper>(ref, valid_size)) {}

AudioTaskBuffer::AudioTaskBuffer() : _storage(StringStorage{}) {}

AudioTaskBuffer::AudioTaskBuffer(std::string&& str)
    : _storage(StringStorage(std::make_shared<std::string>(std::move(str)))) {}

AudioTaskBuffer::AudioTaskBuffer(AVBufferRef* ref, uint64_t valid_size) : _storage(AVBufferRefStorage(ref, valid_size)) {}

std::pair<const char*, size_t> AudioTaskBuffer::_storage_view() const {
    return std::visit(
        [](const auto& storage) -> std::pair<const char*, size_t> {
            using Type = std::decay_t<decltype(storage)>;

            if constexpr (std::is_same_v<Type, StringStorage>) {
                return {storage._shared_ref->data(), storage._shared_ref->size()};
            } else if constexpr (std::is_same_v<Type, AVBufferRefStorage>) {
                return {
                    reinterpret_cast<const char*>(storage._shared_ref->_ref->data),
                    static_cast<size_t>(storage._shared_ref->_valid_size),
                };
            }
        },
        _storage);
}

void AudioTaskBuffer::_reset() noexcept {
    _storage = StringStorage{};
    _offset = 0;
}

std::string_view AudioTaskBuffer::str() const {
    const auto [data, size] = _storage_view();
    if (data == nullptr || size <= _offset) {
        return {};
    }
    return {data + _offset, size - _offset};
}

uint8_t AudioTaskBuffer::at(size_t n) const {
    const auto str_view = str();
    if (n >= str_view.size()) {
        throw std::out_of_range("AudioTaskBuffer::at");
    }
    return static_cast<uint8_t>(str_view[n]);
}

size_t AudioTaskBuffer::size() const {
    return str().size();
}

std::string AudioTaskBuffer::copy() const {
    return std::string(str());
}

void AudioTaskBuffer::remove_prefix(size_t n) {
    if (n > size()) {
        throw std::out_of_range("AudioTaskBuffer::remove_prefix");
    }

    _offset += n;

    const auto [data, total] = _storage_view();
    (void)data;
    if (_offset >= total) {
        _reset();
    }
}

AudioTaskBufferList::AudioTaskBufferList(AudioTaskBuffer buffer) {
    _buffers.push_back(std::move(buffer));
}

AudioTaskBufferList::AudioTaskBufferList(std::string&& str) {
    _buffers.emplace_back(std::move(str));
}

AudioTaskBufferList::AudioTaskBufferList(AVBufferRef* ref, uint64_t valid_size) : _buffers{AudioTaskBuffer(ref, valid_size)} {}

void AudioTaskBufferList::append(const AudioTaskBufferList& other) {
    _buffers.insert(_buffers.end(), other._buffers.begin(), other._buffers.end());
}

void AudioTaskBufferList::append(AudioTaskBufferList&& other) {
    _buffers.insert(
        _buffers.end(),
        std::make_move_iterator(other._buffers.begin()),
        std::make_move_iterator(other._buffers.end()));
    other._buffers.clear();
}

AudioTaskBufferList::operator AudioTaskBuffer() const {
    switch (_buffers.size()) {
        case 0:
            return {};
        case 1:
            return _buffers.front();
        default:
            throw std::runtime_error(
                "AudioTaskBufferList contains multiple buffers; use concatenate() "
                "to combine them into one contiguous buffer");
    }
}

void AudioTaskBufferList::remove_prefix(size_t n) {
    if (n > size()) {
        throw std::out_of_range("AudioTaskBufferList::remove_prefix");
    }

    while (n > 0) {
        AudioTaskBuffer& front = _buffers.front();
        const size_t front_size = front.size();

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
    for (const auto& buf : _buffers) {
        total += buf.size();
    }
    return total;
}

std::string AudioTaskBufferList::concatenate() const {
    std::string result;
    result.reserve(size());

    for (const auto& buf : _buffers) {
        result.append(buf.str());
    }

    return result;
}

std::string AudioTaskBufferList::concatenate(size_t max_len) const {
    std::string result;
    result.reserve(std::min(max_len, size()));

    size_t remaining = max_len;
    for (const auto& buf : _buffers) {
        if (remaining == 0) {
            break;
        }

        const size_t to_copy = std::min(remaining, buf.size());
        result.append(buf.str().substr(0, to_copy));
        remaining -= to_copy;
    }

    return result;
}

const AudioTaskBuffer& AudioTaskBufferList::front() const {
    if (_buffers.empty()) {
        throw std::out_of_range("AudioTaskBufferList::front on empty list");
    }
    return _buffers.front();
}

AudioTaskBuffer AudioTaskBufferList::pop_front() {
    if (_buffers.empty()) {
        throw std::out_of_range("AudioTaskBufferList::pop_front on empty list");
    }

    AudioTaskBuffer buffer = std::move(_buffers.front());
    _buffers.pop_front();
    return buffer;
}

AudioTaskBufferViewList::AudioTaskBufferViewList(const std::string& str)
    : AudioTaskBufferViewList(std::string_view(str)) {}

AudioTaskBufferViewList::AudioTaskBufferViewList(const char* s)
    : AudioTaskBufferViewList(std::string_view(s)) {}

AudioTaskBufferViewList::AudioTaskBufferViewList(const AudioTaskBufferList& buffers) {
    for (const auto& x : buffers.buffers()) {
        _views.push_back(x.str());
    }
}

AudioTaskBufferViewList::AudioTaskBufferViewList(std::string_view str) {
    _views.push_back(str);
}

void AudioTaskBufferViewList::append(std::string_view str_view) {
    _views.push_back(str_view);
}

void AudioTaskBufferViewList::remove_prefix(size_t n) {
    if (n > size()) {
        throw std::out_of_range("AudioTaskBufferViewList::remove_prefix");
    }

    while (n > 0) {
        std::string_view& front = _views.front();

        if (n < front.size()) {
            front.remove_prefix(n);
            return;
        }

        n -= front.size();
        _views.pop_front();
    }
}

size_t AudioTaskBufferViewList::size() const {
    size_t total = 0;
    for (const auto& buf : _views) {
        total += buf.size();
    }
    return total;
}

}  // namespace audiotask::core
