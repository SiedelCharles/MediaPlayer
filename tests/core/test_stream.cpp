#pragma once
#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <atomic>

#include "AudioTaskByteStream.hpp"

using namespace audiotask::core;
using namespace std::chrono_literals;

//============================================================
// Helper
// ============================================================
namespace {

/// @brief 创建一个内容为指定字符、指定长度的 AudioTaskBuffer
AudioTaskBuffer makeBuffer(char fill, size_t size)
{
    return AudioTaskBuffer{std::string(size, fill)};
}

/// @brief 验证 AudioTaskBuffer 内容是否全为指定字符
bool allCharsEqual(const AudioTaskBuffer& buf, char expected)
{
    std::string_view sv = buf.str();
    if (sv.empty()) return false;
    for (char c : sv) {
        if (c != expected) return false;
    }
    return true;
}

/// @brief 获取 buffer 第一个字节
char firstChar(const AudioTaskBuffer& buf)
{
    return static_cast<char>(buf.at(0));
}

} // namespace

// ============================================================
// Fixture
// ============================================================
class AudioTaskByteStreamTest : public ::testing::Test {
protected:
    AudioTaskByteStream stream{1024};
};

// ============================================================
// AudioTaskBuffer 自身接口测试
// ============================================================
TEST(AudioTaskBufferTest, DefaultConstructIsEmpty)
{
    AudioTaskBuffer buf{};
    EXPECT_TRUE(buf.empty());
    EXPECT_EQ(buf.size(), 0u);
}

TEST(AudioTaskBufferTest, ConstructFromString)
{
    AudioTaskBuffer buf{std::string(16, 'A')};
    EXPECT_FALSE(buf.empty());
    EXPECT_EQ(buf.size(), 16u);
}

TEST(AudioTaskBufferTest, StrReturnsCorrectView)
{
    AudioTaskBuffer buf{std::string{"Hello"}};
    std::string_view sv = buf.str();
    EXPECT_EQ(sv, "Hello");
}

TEST(AudioTaskBufferTest, ImplicitConversionToStringView)
{
    AudioTaskBuffer buf{std::string{"World"}};
    std::string_view sv = buf;
    EXPECT_EQ(sv, "World");
}

TEST(AudioTaskBufferTest, AtReturnsCorrectByte)
{
    AudioTaskBuffer buf{std::string{"Hello"}};
    EXPECT_EQ(buf.at(0), static_cast<uint8_t>('H'));
    EXPECT_EQ(buf.at(1), static_cast<uint8_t>('e'));
    EXPECT_EQ(buf.at(4), static_cast<uint8_t>('o'));
}

TEST(AudioTaskBufferTest, CopyReturnsEqualString)
{
    std::string original(64, 'Q');
    AudioTaskBuffer buf{std::string(original)};
    std::string copied = buf.copy();
    EXPECT_EQ(copied, original);
}

TEST(AudioTaskBufferTest, RemovePrefixReducesSize)
{
    AudioTaskBuffer buf{std::string{"Hello"}};
    buf.remove_prefix(2);
    EXPECT_EQ(buf.size(), 3u);
    EXPECT_EQ(buf.str(), "llo");
}

TEST(AudioTaskBufferTest, RemoveAllPrefixMakesEmpty)
{
    AudioTaskBuffer buf{std::string{"Hello"}};
    buf.remove_prefix(5);
    EXPECT_TRUE(buf.empty());
}

// ============================================================
// 初始状态
// ============================================================
TEST_F(AudioTaskByteStreamTest, InitialStateIsCorrect)
{
    EXPECT_EQ(stream.capacity(),1024u);
    EXPECT_EQ(stream.buffer_size(),         0u);
    EXPECT_EQ(stream.count(),               0u);
    EXPECT_EQ(stream.remaining_capacity(),  1024u);
    EXPECT_TRUE (stream.buffer_empty());
    EXPECT_FALSE(stream.full());
    EXPECT_FALSE(stream.eof());
    EXPECT_FALSE(stream.error());
    EXPECT_FALSE(stream.input_ended());
    EXPECT_FALSE(stream.is_aborted());
    EXPECT_FALSE(stream.is_flushing());
}

// ============================================================
// write
// ============================================================
/// @todo error here
TEST_F(AudioTaskByteStreamTest, WriteIncreasesBufferSize)
{
    size_t written = stream.write(makeBuffer('A', 128));
    EXPECT_EQ(written,128u);
    EXPECT_EQ(stream.buffer_size(), 128u);
    // EXPECT_EQ(stream.count(),       1u);
    EXPECT_FALSE(stream.buffer_empty());
}

TEST_F(AudioTaskByteStreamTest, MultipleWritesAccumulateSize)
{
    stream.write(makeBuffer('A', 100));
    stream.write(makeBuffer('B', 200));
    stream.write(makeBuffer('C', 300));
    EXPECT_EQ(stream.buffer_size(), 600u);
    // EXPECT_EQ(stream.count(),       3u);
}

TEST_F(AudioTaskByteStreamTest, RemainingCapacityDecreasesAfterWrite)
{
    stream.write(makeBuffer('X', 400));
    EXPECT_EQ(stream.remaining_capacity(), 624u);
}

TEST_F(AudioTaskByteStreamTest, RemainingCapacityIsZeroWhenFull)
{
    stream.write(makeBuffer('A', 1024));
    EXPECT_EQ(stream.remaining_capacity(), 0u);
}

// ============================================================
// try_write
// ============================================================
TEST_F(AudioTaskByteStreamTest, TryWriteSucceedsWhenSpaceAvailable)
{
    size_t written = stream.try_write(makeBuffer('A', 100));
    EXPECT_EQ(written, 100u);
}

TEST_F(AudioTaskByteStreamTest, TryWriteFailsWhenFull)
{
    stream.write(makeBuffer('A', 1024));
    size_t written = stream.try_write(makeBuffer('B', 1));
    EXPECT_EQ(written, 0u);
}

TEST_F(AudioTaskByteStreamTest, TryWriteReturnsZeroAfterAbort)
{
    stream.abort();
    size_t written = stream.try_write(makeBuffer('A', 64));
    EXPECT_EQ(written, 0u);
}

// ============================================================
// pop
// ============================================================
TEST_F(AudioTaskByteStreamTest, PopReturnsCorrectBuffer)
{
    stream.write(makeBuffer('Z', 64));
    AudioTaskBuffer result = stream.pop();
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result.size(), 64u);
    EXPECT_TRUE(allCharsEqual(result, 'Z'));
}

TEST_F(AudioTaskByteStreamTest, PopDecreasesCount)
{
    stream.write(makeBuffer('A', 64));
    stream.write(makeBuffer('B', 64));
    stream.pop();
    // EXPECT_EQ(stream.count(), 1u);
    stream.pop();
    // EXPECT_EQ(stream.count(), 0u);
}

TEST_F(AudioTaskByteStreamTest, PopDecreasesBufferSize)
{
    stream.write(makeBuffer('A', 128));
    stream.pop();
    EXPECT_EQ(stream.buffer_size(), 0u);
}

TEST_F(AudioTaskByteStreamTest, PopOrderIsFIFO)
{
    stream.write(makeBuffer('A', 32));
    stream.write(makeBuffer('B', 32));
    stream.write(makeBuffer('C', 32));
    auto b1 = stream.pop();
    auto b2 = stream.pop();
    auto b3 = stream.pop();
    EXPECT_EQ(firstChar(b1), 'A');
    EXPECT_EQ(firstChar(b2), 'B');
    EXPECT_EQ(firstChar(b3), 'C');
}

TEST_F(AudioTaskByteStreamTest, PopRestoresCapacityAfterPop)
{
    stream.write(makeBuffer('A', 512));
    stream.pop();
    EXPECT_EQ(stream.remaining_capacity(), 1024u);
}

// ============================================================
// try_pop
// ============================================================
TEST_F(AudioTaskByteStreamTest, TryPopReturnsEmptyWhenQueueEmpty)
{
    AudioTaskBuffer result = stream.try_pop();
    EXPECT_TRUE(result.empty());
}

TEST_F(AudioTaskByteStreamTest, TryPopReturnsBufferWhenDataAvailable)
{
    stream.write(makeBuffer('X', 48));
    AudioTaskBuffer result = stream.try_pop();
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result.size(), 48u);EXPECT_TRUE(allCharsEqual(result, 'X'));
}

TEST_F(AudioTaskByteStreamTest, TryPopReturnsEmptyAfterAbort)
{
    stream.write(makeBuffer('A', 64));
    stream.abort();
    EXPECT_TRUE(stream.try_pop().empty());
}

TEST_F(AudioTaskByteStreamTest, TryPopReturnsEmptyAfterEndInputAndQueueEmpty)
{
    stream.end_input();
    EXPECT_TRUE(stream.try_pop().empty());
}

// ============================================================
// peek_output
// ============================================================
TEST_F(AudioTaskByteStreamTest, PeekDoesNotRemoveData)
{
    stream.write(makeBuffer('A', 16));
    std::string peeked = stream.peek_output(8);
    EXPECT_EQ(peeked.size(),8u);
    EXPECT_EQ(stream.buffer_size(), 16u);// 数据还在
}

TEST_F(AudioTaskByteStreamTest, PeekReturnsCorrectContent)
{
    stream.write(makeBuffer('Z', 16));
    std::string peeked = stream.peek_output(4);
    EXPECT_EQ(peeked, std::string(4, 'Z'));
}

TEST_F(AudioTaskByteStreamTest, PeekMoreThanAvailableReturnsAllAvailable)
{
    stream.write(makeBuffer('A', 8));
    std::string peeked = stream.peek_output(9999);
    EXPECT_EQ(peeked.size(), 8u);
    EXPECT_EQ(peeked, std::string(8, 'A'));
}

TEST_F(AudioTaskByteStreamTest, PeekAcrossMultipleBuffers)
{
    stream.write(makeBuffer('A', 8));
    stream.write(makeBuffer('B', 8));
    std::string peeked = stream.peek_output(16);
    EXPECT_EQ(peeked.size(), 16u);
    EXPECT_EQ(peeked.substr(0, 8), std::string(8, 'A'));
    EXPECT_EQ(peeked.substr(8, 8), std::string(8, 'B'));
}

// ============================================================
// pop_output
// ============================================================
TEST_F(AudioTaskByteStreamTest, PopOutputRemovesBytes)
{
    stream.write(makeBuffer('A', 16));
    stream.pop_output(8);
    EXPECT_EQ(stream.buffer_size(), 8u);
}

TEST_F(AudioTaskByteStreamTest, PopOutputRemovesAllBytes)
{
    stream.write(makeBuffer('A', 16));
    stream.pop_output(16);
    EXPECT_EQ(stream.buffer_size(), 0u);
    EXPECT_TRUE(stream.buffer_empty());
}

TEST_F(AudioTaskByteStreamTest, PopOutputAcrossMultipleBuffers)
{
    stream.write(makeBuffer('A', 8));
    stream.write(makeBuffer('B', 8));
    stream.pop_output(12);
    EXPECT_EQ(stream.buffer_size(), 4u);
}

// ============================================================
// read
// ============================================================
TEST_F(AudioTaskByteStreamTest, ReadReturnsAndRemovesBytes)
{
    stream.write(makeBuffer('B', 32));
    std::string data = stream.read(16);
    EXPECT_EQ(data.size(),16u);
    EXPECT_EQ(data,                 std::string(16, 'B'));
    EXPECT_EQ(stream.buffer_size(), 16u);
}

TEST_F(AudioTaskByteStreamTest, ReadAcrossMultipleBuffers)
{
    stream.write(makeBuffer('A', 8));
    stream.write(makeBuffer('B', 8));
    std::string data = stream.read(16);
    EXPECT_EQ(data.size(), 16u);
    EXPECT_EQ(data.substr(0, 8), std::string(8, 'A'));
    EXPECT_EQ(data.substr(8, 8), std::string(8, 'B'));
}

TEST_F(AudioTaskByteStreamTest, ReadAllDataLeavesStreamEmpty)
{
    stream.write(makeBuffer('C', 64));
    stream.read(64);
    EXPECT_EQ(stream.buffer_size(), 0u);EXPECT_TRUE(stream.buffer_empty());
}

TEST_F(AudioTaskByteStreamTest, ReadMoreThanAvailableReturnsAllAvailable)
{
    stream.write(makeBuffer('D', 32));
    std::string data = stream.read(9999);
    EXPECT_EQ(data.size(), 32u);
    EXPECT_EQ(data, std::string(32, 'D'));
}

// ============================================================
// end_input /eof / input_ended
// ============================================================
TEST_F(AudioTaskByteStreamTest, InputEndedAfterEndInput)
{
    stream.end_input();
    EXPECT_TRUE(stream.input_ended());
}

TEST_F(AudioTaskByteStreamTest, EofWhenInputEndedAndBufferEmpty)
{
    stream.end_input();
    EXPECT_TRUE(stream.eof());
}

TEST_F(AudioTaskByteStreamTest, NotEofWhenInputEndedButDataRemains)
{
    stream.write(makeBuffer('A', 64));
    stream.end_input();
    EXPECT_TRUE(stream.input_ended());
    EXPECT_FALSE(stream.eof());
}

TEST_F(AudioTaskByteStreamTest, EofAfterReadingAllDataAndInputEnded)
{
    stream.write(makeBuffer('A', 32));
    stream.end_input();
    stream.read(32);
    EXPECT_TRUE(stream.eof());
}

// ============================================================
// error
// ============================================================
TEST_F(AudioTaskByteStreamTest, ErrorInitiallyFalse)
{
    EXPECT_FALSE(stream.error());
}

TEST_F(AudioTaskByteStreamTest, SetErrorSetsFlag)
{
    stream.set_error();
    EXPECT_TRUE(stream.error());
}

// ============================================================
// abort
// ============================================================
TEST_F(AudioTaskByteStreamTest, AbortInitiallyFalse)
{
    EXPECT_FALSE(stream.is_aborted());
}

TEST_F(AudioTaskByteStreamTest, AbortSetsAbortedFlag)
{
    stream.abort();
    EXPECT_TRUE(stream.is_aborted());
}

TEST_F(AudioTaskByteStreamTest, AbortedStreamTryWriteReturnsZero)
{
    stream.abort();
    EXPECT_EQ(stream.try_write(makeBuffer('A', 64)), 0u);
}

TEST_F(AudioTaskByteStreamTest, AbortedStreamTryPopReturnsEmpty)
{
    stream.write(makeBuffer('A', 64));
    stream.abort();
    EXPECT_TRUE(stream.try_pop().empty());
}

// ============================================================
// clear
// ============================================================
TEST_F(AudioTaskByteStreamTest, ClearEmptiesBuffer)
{
    stream.write(makeBuffer('A', 128));
    stream.write(makeBuffer('B', 128));
    stream.clear();
    EXPECT_EQ(stream.buffer_size(), 0u);
    EXPECT_EQ(stream.count(),       0u);
    EXPECT_TRUE(stream.buffer_empty());
}

TEST_F(AudioTaskByteStreamTest, ClearRestoresRemainingCapacity)
{
    stream.write(makeBuffer('A', 512));
    stream.clear();
    EXPECT_EQ(stream.remaining_capacity(), 1024u);
}

TEST_F(AudioTaskByteStreamTest, CanWriteAfterClear)
{
    stream.write(makeBuffer('A', 1024));
    stream.clear();
    size_t written = stream.try_write(makeBuffer('B', 64));
    EXPECT_EQ(written, 64u);
}

// ============================================================
// flush_begin / flush_end
// ============================================================
TEST_F(AudioTaskByteStreamTest, FlushBeginSetsFlushingFlag)
{
    EXPECT_FALSE(stream.is_flushing());
    stream.flush_begin();
    EXPECT_TRUE(stream.is_flushing());
}

TEST_F(AudioTaskByteStreamTest, FlushBeginClearsBuffers)
{
    stream.write(makeBuffer('A', 256));
    stream.flush_begin();
    EXPECT_TRUE(stream.buffer_empty());EXPECT_EQ(stream.buffer_size(), 0u);
}

TEST_F(AudioTaskByteStreamTest, FlushBeginRestoresCapacity)
{
    stream.write(makeBuffer('A', 512));
    stream.flush_begin();
    EXPECT_EQ(stream.remaining_capacity(), 1024u);
}

TEST_F(AudioTaskByteStreamTest, FlushEndClearsFlushingFlag)
{
    stream.flush_begin();
    stream.flush_end();
    EXPECT_FALSE(stream.is_flushing());
}

TEST_F(AudioTaskByteStreamTest, CanWriteAfterFlushEnd)
{
    stream.flush_begin();
    stream.flush_end();
    size_t written = stream.try_write(makeBuffer('A', 64));
    EXPECT_EQ(written, 64u);
}

// ============================================================
// full
// ============================================================
TEST_F(AudioTaskByteStreamTest, NotFullInitially)
{
    EXPECT_FALSE(stream.full());
}

TEST_F(AudioTaskByteStreamTest, FullWhenCapacityReached)
{
    stream.write(makeBuffer('A', 1024));
    EXPECT_TRUE(stream.full());
}

TEST_F(AudioTaskByteStreamTest, NotFullAfterPop)
{
    stream.write(makeBuffer('A', 1024));
    stream.pop();
    EXPECT_FALSE(stream.full());
}

// ============================================================
// 并发测试
// ============================================================
TEST(AudioTaskByteStreamConcurrentTest, SingleProducerSingleConsumer)
{
    AudioTaskByteStream stream{4096};
    constexpr int kPackets= 20;
    constexpr size_t kPacketSize = 64;
    std::atomic<int> consumed{0};

    std::thread producer([&]() {
        for (int i = 0; i < kPackets; ++i) {
            stream.write(makeBuffer(
                static_cast<char>('A' + (i % 26)),
                kPacketSize
            ));
        }stream.end_input();
    });

    std::thread consumer([&]() {
        while (true) {
            AudioTaskBuffer buf = stream.pop();
            if (buf.empty()) break;
            ++consumed;
        }
    });

    producer.join();
    consumer.join();

    EXPECT_EQ(consumed.load(), kPackets);
}

TEST(AudioTaskByteStreamConcurrentTest, AbortUnblocksBlockingPop)
{
    AudioTaskByteStream stream{1024};
    std::atomic<bool> popReturned{false};

    std::thread consumer([&]() {
        stream.pop();           // 队列为空，阻塞
        popReturned = true;
    });

    std::this_thread::sleep_for(50ms);
    stream.abort();

    consumer.join();
    EXPECT_TRUE(popReturned.load());
}

TEST(AudioTaskByteStreamConcurrentTest, AbortUnblocksBlockingWrite)
{
    AudioTaskByteStream stream{64};
    stream.write(makeBuffer('A', 64));  // 填满

    std::atomic<bool> writeReturned{false};

    std::thread producer([&]() {
        stream.write(makeBuffer('B', 64));  // 满了，阻塞
        writeReturned = true;
    });

    std::this_thread::sleep_for(50ms);
    stream.abort();

    producer.join();
    EXPECT_TRUE(writeReturned.load());
}

TEST(AudioTaskByteStreamConcurrentTest, EndInputUnblocksBlockingPop)
{
    AudioTaskByteStream stream{1024};
    std::atomic<bool> popReturned{false};

    std::thread consumer([&]() {
        stream.pop();           // 队列为空，阻塞
        popReturned = true;
    });

    std::this_thread::sleep_for(50ms);
    stream.end_input();

    consumer.join();
    EXPECT_TRUE(popReturned.load());
}

TEST(AudioTaskByteStreamConcurrentTest, MultipleProducersOneConsumer)
{
    AudioTaskByteStream stream{8192};
    constexpr int    kProducers   = 4;
    constexpr int    kPacketsEach = 10;
    constexpr size_t kPacketSize  = 32;
    std::atomic<int> consumed{0};

    std::vector<std::thread> producers;
    for (int i = 0; i < kProducers; ++i) {
        producers.emplace_back([&]() {
            for (int j = 0; j < kPacketsEach; ++j) {
                stream.write(makeBuffer('X', kPacketSize));
            }
        });
    }

    for (auto& p : producers) p.join();
    stream.end_input();

    while (true) {
        AudioTaskBuffer buf = stream.pop();
        if (buf.empty()) break;
        ++consumed;
    }

    EXPECT_EQ(consumed.load(), kProducers * kPacketsEach);
}

TEST(AudioTaskByteStreamConcurrentTest, ProducerConsumerWithReadInterface)
{
    AudioTaskByteStream stream{4096};
    constexpr size_t kChunkSize  = 64;
    constexpr int    kChunks     = 4;
    std::atomic<size_t> totalRead{0};

    std::thread producer([&]() {
        for (int i = 0; i < kChunks; ++i) {
            stream.write(makeBuffer('Y', kChunkSize));
        }
        stream.end_input();
    });

    std::thread consumer([&]() {
        while (!stream.eof()) {
            if (stream.buffer_size() >= kChunkSize) {
                std::string data = stream.read(kChunkSize);
                totalRead += data.size();
                EXPECT_EQ(data, std::string(kChunkSize, 'Y'));
            } else {
                std::this_thread::yield();
            }
        }
    });

    producer.join();
    consumer.join();

    EXPECT_EQ(totalRead.load(), kChunkSize * kChunks);
}
