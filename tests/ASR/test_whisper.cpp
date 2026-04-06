#include "WhisperAsr/WhisperAsr.hpp"
#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <memory>

#include "sources/FileSource/FFmpegFileSource.hpp"
#include "VadFrame/WebRtcVadFrame.hpp"
#include "AudioTaskVad.hpp"
#include "AudioTaskPipeline.hpp"
#include "AudioTaskQueue.hpp"

const std::string source_file_path = "D:\\VisualStudio_Created\\VisualStudio_Project\\Projects\\MediaPlayer\\resources\\pictures\\ts\\TS\\mp3\\03.mp3";
const std::string model_path = "D:\\VisualStudio_Created\\VisualStudio_Resource\\audio\\whisper.cpp\\models\\ggml-small.bin";

using namespace audiotask;
int main() {
    core::AudioTaskPipeline pipeline;

    std::shared_ptr<source::FileSource> source = std::make_shared<source::FFmpegFileSource>();
    source::SourceFormat out_fmt;
    out_fmt.channels = 1;
    out_fmt.sample_rate = 16000;
    out_fmt.format = source::SampleFormat::S16;
    source->set_output_format(out_fmt);
    source->init(source_file_path);

    std::unique_ptr<vad::VadFrame> webrtc_frame = std::make_unique<vad::WebRtcVadFrame>();
    std::shared_ptr<vad::AudioTaskVad> vad = std::make_shared<vad::AudioTaskVad>(std::move(webrtc_frame));

    
    std::shared_ptr<asr::WhisperAsr> asr = std::make_shared<asr::WhisperAsr>();
    whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    params.print_realtime   = false;
    params.print_progress   = false;
    params.print_timestamps = false;
    params.print_special    = false;
    params.translate        = false;
    params.language         = "ja";
    params.n_threads        = 4;
    params.offset_ms        = 0;
    params.duration_ms      = 0;
    params.single_segment   = true;
    std::string prompt = "これは日本語の成人向けASMR音声です。登場人物はマンドラベーゼ、キュラピンク、ポルルン。魔法少女、妖精、悪の幹部、プリンプリンクリスタル。触手、種付け、性転換、産卵、潮吹き、苗床。";
    params.initial_prompt = prompt.data();
    asr->init(model_path, params);

    uint64_t capacity = 1ull << 30;
    std::shared_ptr<core::AudioTaskQueue> buffer_stream = std::make_shared<core::AudioTaskQueue>(capacity);

    pipeline.add(source);
    pipeline.add(vad);
    pipeline.add(asr);
    pipeline.add(buffer_stream);
    
    pipeline.link(source.get(), vad.get());
    pipeline.link(vad.get(), buffer_stream.get());
    pipeline.link(buffer_stream.get(), asr.get());

    auto is_start = pipeline.start();
    std::this_thread::sleep_for(std::chrono::minutes(30));

    return 0;
}