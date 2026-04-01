#include "AudioTaskByteStream.hpp"
namespace audiotask::core {
size_t AudioTaskByteStream::write(AudioTaskBuffer buffer) {
    if (buffer.empty()) return 0;
    const size_t bytes = buffer.size();
    std::unique_lock<std::mutex> lock(_mutex);
    while (true) {
        if (_aborted or _error or _flushing) return 0;
        if (_current_size + bytes <= _capacity) {
            _current_size += bytes;
            _buffers.append(AudioTaskBufferList(buffer));
            _condition_read.notify_one();
            return bytes;
        }
        _condition_write.wait(lock);
    }
}
size_t AudioTaskByteStream::try_write(AudioTaskBuffer buffer) {
    if (buffer.empty()) return 0;
    const size_t bytes = buffer.size();
    std::lock_guard<std::mutex> lock(_mutex);
    if (_aborted || _error || _flushing) return 0;
    if (_current_size + bytes > _capacity) return 0;
    _current_size += bytes;
    _buffers.append(AudioTaskBufferList(buffer));
    _condition_read.notify_one();
    return bytes;
}
void AudioTaskByteStream::end_input() noexcept {
    std::lock_guard<std::mutex> lock(_mutex);
    _finished = true;
    _condition_read.notify_all();
}
std::string AudioTaskByteStream::peek_output(size_t len) const {
    std::lock_guard<std::mutex> lock(_mutex);
    if (len == 0 || _buffers.empty()) return {};
    return _buffers.concatenate(len);
}
void AudioTaskByteStream::pop_output(size_t len) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (len == 0 or _buffers.empty()) return;
    const size_t to_remove = std::min(len, _current_size);
    _buffers.remove_prefix(to_remove);
    _current_size -= to_remove;
    _condition_write.notify_one();
}
/// @todo supplement pop/peek which return Buffers
AudioTaskBuffer AudioTaskByteStream::pop() {
    std::unique_lock<std::mutex> lock(_mutex);
    while (true) {
        if (_aborted || _flushing) return {};
        if (!_buffers.empty()) {
            auto buffer = _buffers.pop_front();
            _current_size -= buffer.size();
            _condition_write.notify_one();
            return buffer;
        }
        if (_finished) return {};
        _condition_read.wait(lock);
    }
}
AudioTaskBuffer AudioTaskByteStream::try_pop() {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_aborted || _flushing || _buffers.empty()) return AudioTaskBuffer{};
    auto buffer = AudioTaskBuffer(_buffers.buffers().front());
    const size_t size = buffer.size();
    _buffers.remove_prefix(size);
    _current_size -= size;
    _condition_write.notify_one();
    return buffer;
}
void AudioTaskByteStream::abort() noexcept {
    std::lock_guard<std::mutex> lock(_mutex);
    _aborted = true;
    _condition_read.notify_all();
    _condition_write.notify_all();
}
void AudioTaskByteStream::clear() noexcept {
    std::lock_guard<std::mutex> lock(_mutex);
    _buffers = AudioTaskBufferList{};
    _current_size = 0;
    _condition_write.notify_all();
}
void AudioTaskByteStream::flush_begin() noexcept {
    std::lock_guard<std::mutex> lock(_mutex);
    _flushing = true;
    _buffers = AudioTaskBufferList{};
    _current_size = 0;
    _condition_read.notify_all();
    _condition_write.notify_all();
}
void AudioTaskByteStream::flush_end() noexcept {
    std::lock_guard<std::mutex> lock(_mutex);
    _flushing = false;
    _finished = false;
    _condition_read.notify_all();
    _condition_write.notify_all();
}
}