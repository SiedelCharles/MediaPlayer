#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <memory>

#include "sources/FileSource/FFmpegFileSource.hpp"
#include "VadFrame/WebRtcVadFrame.hpp"
#include "AudioTaskVad.hpp"

using namespace audiotask::source;
using namespace audiotask::vad;
using namespace audiotask::core;
using namespace std::chrono_literals;

namespace {
    constexpr const char* kValidAudioFile =
        "D:/VisualStudio_Created/VisualStudio_Project/Projects/MediaPlayer/resources/tests/astesia1.wav";
}

// ============================================================
// Fixture
// ============================================================
class AudioTaskVadTest : public ::testing::Test {
protected:
    FFmpegFileSource source;
    std::unique_ptr<AudioTaskVad> vad;

    void SetUp() override {
        Config config{16000, 20, 2};
        auto vad_frame = std::make_unique<WebRtcVadFrame>(config);
        vad = std::make_unique<AudioTaskVad>(std::move(vad_frame), "TestVad");
    }

    void TearDown() override {
        source.stop();
        source.close();
        vad.reset();
    }

    bool init_source_for_vad() {
        if (!source.init(kValidAudioFile)) {
            return false;
        }

        // 给 VAD 的输入建议使用：16k / mono / s16
        SourceFormat out_fmt;
        out_fmt.channels = 1;
        out_fmt.sample_rate = 16000;
        out_fmt.format = SampleFormat::S16;

        source.set_output_format(out_fmt);
        return true;
    }
};

// ============================================================
// 构造测试
// ============================================================
TEST_F(AudioTaskVadTest, ConstructSuccessfully) {
    ASSERT_NE(vad, nullptr);
}

// ============================================================
// link 测试
// ============================================================
TEST_F(AudioTaskVadTest, CanLinkSourceToVad) {
    EXPECT_TRUE(source.link_to(vad.get()));
}

// ============================================================
// 基本运行测试
// ============================================================
TEST_F(AudioTaskVadTest, CanRunPipeline) {
    ASSERT_TRUE(source.link_to(vad.get()));
    ASSERT_TRUE(init_source_for_vad());

    EXPECT_TRUE(source.start());

    std::this_thread::sleep_for(300ms);

    // 这里只验证整条链能正常跑，不崩溃
    // stop 可能返回 true/false，取决于你的实现是否已经自然结束
    source.stop();
    SUCCEED();
}

// ============================================================
// 重复 start/stop 测试
// ============================================================
TEST_F(AudioTaskVadTest, StartStopIsSafe) {
    ASSERT_TRUE(source.link_to(vad.get()));
    ASSERT_TRUE(init_source_for_vad());

    ASSERT_TRUE(source.start());
    std::this_thread::sleep_for(200ms);

    source.stop();
    source.close();

    ASSERT_TRUE(source.init(kValidAudioFile));

    SourceFormat out_fmt;
    out_fmt.channels = 1;
    out_fmt.sample_rate = 16000;
    out_fmt.format = SampleFormat::S16;
    source.set_output_format(out_fmt);

    ASSERT_TRUE(source.start());
    std::this_thread::sleep_for(200ms);

    source.stop();
    SUCCEED();
}

// ============================================================
// 未初始化直接 start
// ============================================================
TEST_F(AudioTaskVadTest, StartWithoutInitReturnsFalse) {
    ASSERT_TRUE(source.link_to(vad.get()));
    EXPECT_FALSE(source.start());
}

// ============================================================
// 不 link 直接 start source
// 这个是否允许，取决于你的框架设计
// 如果你的 source 必须 link 才能工作，就期望 false
// 如果允许 source 独立运行，就改成 SUCCEED()
// ============================================================
TEST_F(AudioTaskVadTest, StartWithoutLink) {
    ASSERT_TRUE(init_source_for_vad());

    // 根据你的实现决定这里该怎么断言
    // 常见情况：即使没 link，也可以 start
    bool started = source.start();
    if (started) {
        std::this_thread::sleep_for(200ms);
        source.stop();
    }

    SUCCEED();
}
