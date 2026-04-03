#pragma once
// FFmpegFileSourceTest.hpp
#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include "sources/FileSource/FFmpegFileSource.hpp"

using namespace audiotask::source;
using namespace audiotask::core;
using namespace std::chrono_literals;

//============================================================
// 测试资源路径
// ============================================================
namespace {
    constexpr const char* kValidAudioFile = "D:/VisualStudio_Created/VisualStudio_Project/Projects/MediaPlayer/resources/tests/astesia1.wav";
    constexpr const char* kInvalidFile= "/no/such/file.mp3";
    constexpr const char* kEmptyPath      = "";
}

// ============================================================
//辅助：构造一个接收端 Pad，挂 chain_func，收集数据
// ============================================================
class MockSink : public AudioTaskElement {
public:
    std::vector<AudioTaskBuffer> received;
    std::atomic<bool> ended{false};

    explicit MockSink() : AudioTaskElement("MockSink") {
        auto* pad = add_pad(Direction::Receiving);
        pad->set_chain_function([this](AudioTaskBufferList&& list) -> FlowReturn {
            // 把收到的每个 buffer 存起来
            while (!list.empty()) {
                received.push_back(list.pop_front());
            }
            return FlowReturn::Successful;
        });
    }

    size_t total_bytes() const {
        size_t n = 0;
        for (auto& b : received) n += b.size();
        return n;
    }
};

// ============================================================
// Fixture
// ============================================================
class FFmpegFileSourceTest : public ::testing::Test {
protected:
    FFmpegFileSource source;
    MockSink         sink;

    void SetUp() override {
        // 把source 的 sending pad 链接到 sink 的 receiving pad
        ASSERT_TRUE(source.link_to(&sink));
    }

    void TearDown() override {
        source.stop();
        source.close();
    }

    // 便捷：init + 设置输出格式 + start，等待最多 ms毫秒
    bool run_source(int wait_ms = 3000) {
        if (!source.init(kValidAudioFile)) return false;
        // 指定输出格式：立体声, 44100Hz, F32SourceFormat out_fmt;
        SourceFormat out_fmt;
        out_fmt.channels= 2;
        out_fmt.sample_rate = 44100;
        out_fmt.format      = SampleFormat::F32;
        source.set_output_format(out_fmt);
        if (!source.start()) return false;
        std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms));
        return true;
    }
};

// ============================================================
// init() 测试
// ============================================================
TEST_F(FFmpegFileSourceTest, InitValidFileReturnsTrue)
{
    EXPECT_TRUE(source.init(kValidAudioFile));
    EXPECT_TRUE(source.is_initialized());
}

TEST_F(FFmpegFileSourceTest, InitInvalidFileReturnsFalse)
{
    EXPECT_FALSE(source.init(kInvalidFile));
    EXPECT_FALSE(source.is_initialized());
}

TEST_F(FFmpegFileSourceTest, InitEmptyPathReturnsFalse)
{
    EXPECT_FALSE(source.init(kEmptyPath));
    EXPECT_FALSE(source.is_initialized());
}

TEST_F(FFmpegFileSourceTest, InitSetsFilePath)
{
    ASSERT_TRUE(source.init(kValidAudioFile));
    EXPECT_EQ(source.file_path(), kValidAudioFile);
}

TEST_F(FFmpegFileSourceTest, InitSetsSourceFormat)
{
    ASSERT_TRUE(source.init(kValidAudioFile));
    auto fmt = source.source_format();
    EXPECT_GT(fmt.channels,0);
    EXPECT_GT(fmt.sample_rate,  0);
    EXPECT_GT(fmt.duration,     0.0);
    EXPECT_NE(fmt.format, SampleFormat::Undefined);
}

TEST_F(FFmpegFileSourceTest, DoubleInitReturnsFalse)
{
    ASSERT_TRUE(source.init(kValidAudioFile));
    EXPECT_FALSE(source.init(kValidAudioFile));
}

// ============================================================
// close() 测试
// ============================================================
TEST_F(FFmpegFileSourceTest, CloseWithoutInitReturnsFalse)
{
    // 未初始化也不应崩溃
    EXPECT_FALSE(source.close());
    EXPECT_FALSE(source.is_initialized());
}

TEST_F(FFmpegFileSourceTest, CloseAfterInitResetsState)
{
    ASSERT_TRUE(source.init(kValidAudioFile));ASSERT_TRUE(source.close());
    EXPECT_FALSE(source.is_initialized());
}

TEST_F(FFmpegFileSourceTest, DoubleCloseIsSafe)
{
    ASSERT_TRUE(source.init(kValidAudioFile));
    ASSERT_TRUE(source.close());
    EXPECT_FALSE(source.close()); // 第二次也不应崩溃
}

TEST_F(FFmpegFileSourceTest, ReinitAfterCloseSucceeds)
{
    ASSERT_TRUE(source.init(kValidAudioFile));
    ASSERT_TRUE(source.close());
    EXPECT_TRUE(source.init(kValidAudioFile));
}

// ============================================================
// start() / stop() 测试
// ============================================================
TEST_F(FFmpegFileSourceTest, StartWithoutInitReturnsFalse)
{
    EXPECT_FALSE(source.start());
}

TEST_F(FFmpegFileSourceTest, StartAfterInitReturnsTrue)
{
    ASSERT_TRUE(source.init(kValidAudioFile));
    EXPECT_TRUE(source.start());source.stop();
}

TEST_F(FFmpegFileSourceTest, DoubleStartReturnsFalse)
{
    ASSERT_TRUE(source.init(kValidAudioFile));
    ASSERT_TRUE(source.start());
    EXPECT_FALSE(source.start()); // 已在运行source.stop();
}

TEST_F(FFmpegFileSourceTest, StopWithoutStartReturnsFalse)
{EXPECT_FALSE(source.stop());
}

// ============================================================
// seek() 测试
// ============================================================
TEST_F(FFmpegFileSourceTest, SeekWithoutInitReturnsFalse)
{
    EXPECT_FALSE(source.seek(0.0));
}

TEST_F(FFmpegFileSourceTest, SeekToBeginReturnsTrue)
{
    ASSERT_TRUE(source.init(kValidAudioFile));
    EXPECT_TRUE(source.seek(0.0));
}

TEST_F(FFmpegFileSourceTest, SeekToMiddleReturnsTrue)
{
    ASSERT_TRUE(source.init(kValidAudioFile));
    double mid = source.source_format().duration / 2.0;
    EXPECT_TRUE(source.seek(mid));
}
/// @todo seek
// TEST_F(FFmpegFileSourceTest, SeekBeyondDurationReturnsFalse)
// {
//     ASSERT_TRUE(source.init(kValidAudioFile));
//     double beyond = source.source_format().duration + 100.0;
//     EXPECT_FALSE(source.seek(beyond));
// }

TEST_F(FFmpegFileSourceTest, SeekNegativeReturnsFalse)
{
    ASSERT_TRUE(source.init(kValidAudioFile));
    EXPECT_FALSE(source.seek(-1.0));
}

// ============================================================
// set_output_format() 测试
// ============================================================
TEST_F(FFmpegFileSourceTest, SetOutputFormatBeforeStartTakesEffect)
{
    SourceFormat fmt;
    fmt.channels    = 1;
    fmt.sample_rate = 22050;
    fmt.format      = SampleFormat::S16;
    source.set_output_format(fmt);

    auto result = source.output_format();
    EXPECT_EQ(result.channels,    1);
    EXPECT_EQ(result.sample_rate, 22050);
    EXPECT_EQ(result.format,      SampleFormat::S16);
}

TEST_F(FFmpegFileSourceTest, SetOutputFormatWhileRunningIsIgnored)
{
    ASSERT_TRUE(source.init(kValidAudioFile));

    SourceFormat before;
    before.channels    = 2;
    before.sample_rate = 44100;
    before.format      = SampleFormat::F32;
    source.set_output_format(before);

    ASSERT_TRUE(source.start());

    // 运行中修改应该被忽略
    SourceFormat after;
    after.channels    = 1;
    after.sample_rate = 8000;
    after.format      = SampleFormat::S16;
    source.set_output_format(after);

    auto result = source.output_format();
    EXPECT_EQ(result.channels,    2);
    EXPECT_EQ(result.sample_rate, 44100);
    EXPECT_EQ(result.format,      SampleFormat::F32);

    source.stop();
}

// ============================================================
// run() /数据流测试
// ============================================================
TEST_F(FFmpegFileSourceTest, RunProducesData)
{
    ASSERT_TRUE(run_source(500));
    source.stop();
    EXPECT_GT(sink.total_bytes(), 0u);
}

TEST_F(FFmpegFileSourceTest, RunProducesCorrectSampleFormat)
{
    // 要求输出 F32 立体声 44100
    ASSERT_TRUE(source.init(kValidAudioFile));
    SourceFormat fmt;
    fmt.channels    = 2;
    fmt.sample_rate = 44100;
    fmt.format      = SampleFormat::F32;
    source.set_output_format(fmt);
    ASSERT_TRUE(source.start());

    std::this_thread::sleep_for(200ms);
    source.stop();

    // F32 = 4 bytes/sample, 2ch → 8 bytes/frame
    // 每个buffer 的字节数必须是 8 的倍数
    for (auto& buf : sink.received) {
        EXPECT_EQ(buf.size() % 8, 0u)
            << "buffer size=" << buf.size() << "不是帧对齐的";
    }
}

TEST_F(FFmpegFileSourceTest, RunEndsAfterFullDecode)
{
    ASSERT_TRUE(source.init(kValidAudioFile));
    double duration = source.source_format().duration;
    source.set_output_format({2, 44100, 0.0, SampleFormat::F32});

    ASSERT_TRUE(source.start());

    // 等待比duration 多一点
    std::this_thread::sleep_for(
        std::chrono::milliseconds(static_cast<int>(duration * 1000) + 500));

    // 线程应该已经自然结束
    EXPECT_FALSE(source.stop()); // stop() 返回 false 说明线程已结束
    EXPECT_GT(sink.total_bytes(), 0u);
}
/// @todo seek
// TEST_F(FFmpegFileSourceTest, SeekDuringRunTakesEffect)
// {
//     ASSERT_TRUE(source.init(kValidAudioFile));
//     source.set_output_format({2, 44100, 0.0, SampleFormat::F32});
//     ASSERT_TRUE(source.start());

//     std::this_thread::sleep_for(200ms);
//     size_t bytes_before = sink.total_bytes();

//     // seek 到开头，应该能继续产生数据
//     EXPECT_TRUE(source.seek(0.0));
//     std::this_thread::sleep_for(200ms);EXPECT_GT(sink.total_bytes(), bytes_before);source.stop();
// }

TEST_F(FFmpegFileSourceTest, StopImmediatelyAfterStart)
{
    ASSERT_TRUE(source.init(kValidAudioFile));
    ASSERT_TRUE(source.start());
    EXPECT_TRUE(source.stop()); // 不应死锁或崩溃
}
