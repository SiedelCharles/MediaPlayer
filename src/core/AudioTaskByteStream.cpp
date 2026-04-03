#include "AudioTaskByteStream.hpp"

#include <utility>
#include <algorithm>

namespace audiotask::core {

void AudioTaskByteStream::append(const AudioTaskBufferList& buffers) {
    _buffers.append(buffers);
    _current_size += buffers.size();
}

void AudioTaskByteStream::append(AudioTaskBufferList&& buffers) {
    _current_size += buffers.size();
    _buffers.append(std::move(buffers));
}

AudioTaskBufferViewList AudioTaskByteStream::peek(size_t len) const {
    AudioTaskBufferViewList result{};

    if (len == 0 || _current_size == 0) {
        return result;
    }

    size_t remaining = std::min(len, size());

    for (const auto& buffer : _buffers.buffers()) {
        if (remaining == 0) {
            break;
        }

        const std::string_view str_view = buffer.str();
        const size_t to_take = std::min(remaining, str_view.size());

        if (to_take > 0) {
            result.append(str_view.substr(0, to_take));
            remaining -= to_take;
        }
    }

    return result;
}

std::optional<AudioTaskBufferViewList> AudioTaskByteStream::peek_exact(size_t len) const {
    if (!has(len)) {
        return std::nullopt;
    }

    return peek(len);
}

std::string AudioTaskByteStream::peek_copy(size_t len) const {
    return _buffers.concatenate(len);
}

void AudioTaskByteStream::consume(size_t len) {
    const size_t to_consume = std::min(len, size());

    if (to_consume == 0) {
        return;
    }

    _buffers.remove_prefix(to_consume);
    _current_size -= to_consume;
}

std::string AudioTaskByteStream::read_copy(size_t len) {
    std::string result = peek_copy(len);
    consume(result.size());
    return result;
}

AudioTaskBufferList AudioTaskByteStream::take(size_t len) {
    AudioTaskBufferList result;

    if (len == 0 || _current_size == 0) {
        return result;
    }

    size_t remaining = std::min(len, size());

    while (remaining > 0 && !_buffers.empty()) {
        const AudioTaskBuffer& front = _buffers.front();
        const size_t front_size = front.size();

        if (front_size <= remaining) {
            result.append(AudioTaskBufferList(_buffers.pop_front()));
            remaining -= front_size;
            _current_size -= front_size;
        } else {
            result.append(AudioTaskBufferList(front.copy().substr(0, remaining)));
            _buffers.remove_prefix(remaining);
            _current_size -= remaining;
            remaining = 0;
        }
    }

    return result;
}

void AudioTaskByteStream::clear() noexcept {
    _buffers = AudioTaskBufferList{};
    _current_size = 0;
}

}  // namespace audiotask::core
