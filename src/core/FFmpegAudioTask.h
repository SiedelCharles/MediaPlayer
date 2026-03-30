// #pragma once
// #include "srttimestamp.h"
// #include "AudioTaskInterface.h"

// #include "QtCore/qstringlist"

// #include <span>
// #include <atomic>
// #include <vector>
// #include <cstdint>
// #include <functional>

// extern "C" {
//     #include <libavutil/time.h>
//     #include <libavutil/avutil.h>
//     #include <libavcodec/avcodec.h>
//     #include <libavformat/avformat.h>
//     #include <libswresample/swresample.h>
// }

// struct SwrContextDeleter { ///< deleter for smart pointers
//     void operator()(SwrContext* ctx) const { swr_free(&ctx); }
// };
// struct AVCodecContextDeleter { ///< deleter for smart pointers
//     void operator()(AVCodecContext* ctx) const { avcodec_free_context(&ctx); }
// };
// struct AVFormatContextDeleter { ///< deleter for smart pointers
//     void operator()(AVFormatContext* ctx) const { avformat_close_input(&ctx); }
// };

// /// @brief to register
// static bool bool_meta_type_register = []() {
//     qRegisterMetaType<QSharedPointer<QByteArray>>();
//     return true;
// }(); ///< Add "()" is to solve a known issue in the MSVC compiler regarding the implicit conversion of non-capturing lambdas to bool

// /// @brief audio resampling format
// struct FFmpegFormatConfig{
//     int _sample_rate = 0;
//     int _channel_count = 0;
//     AVSampleFormat _sample_format = AV_SAMPLE_FMT_NONE;
//     std::string _codec_name = "pcm_s16le";
//     FFmpegFormatConfig() = default;
//     FFmpegFormatConfig(int sample_rate, int channel_count, AVSampleFormat sample_format):_sample_rate(sample_rate), _channel_count(channel_count), _sample_format(sample_format){}
//     FFmpegFormatConfig(const FFmpegFormatConfig&) = default;
//     FFmpegFormatConfig(FFmpegFormatConfig&&) noexcept=default;
//     FFmpegFormatConfig& operator=(const FFmpegFormatConfig&)=default;
//     FFmpegFormatConfig& operator=(FFmpegFormatConfig&&) noexcept=default;
//     bool operator==(const FFmpegFormatConfig& other) const {
//         return  _sample_rate == other._sample_rate &&
//                 _channel_count == other._channel_count &&
//                 _sample_format == other._sample_format;
//     }
//     bool operator!=(const FFmpegFormatConfig& other) const {
//         return !(*this==other);
//     }
// };
// enum class FFmpegMergeOption {
//     MergeInsert,
//     MergeMixing,
//     MergeConcatenate,
// };
// class FFmpegAudioTask;
// class FFmpegTaskProcesser {
// public:
//     ~FFmpegTaskProcesser() = default;
//     virtual void play() = 0;
//     virtual bool merge(const std::vector<TimeStampPair>& timestamp_list
//         , const std::vector<std::string> &input_file, const QString& output_file
//         , const FFmpegFormatConfig& config, FFmpegMergeOption option) = 0;
//     virtual bool encode(const FFmpegFormatConfig& config, const QString& output_file) = 0;
//     virtual bool decode(const FFmpegFormatConfig& config, bool is_emit) = 0;
//     virtual bool decode(std::function<void(std::span<const uint8_t>)> f_pcmdata) = 0;
//     virtual bool transcode(const QString& input_file, const QString& output_file) = 0;
// };


// class FFmpegAudioTask : public AudioTaskBase
//                       , public FFmpegTaskProcesser
// {
//     Q_OBJECT
//     using SwrContextPtr     = std::unique_ptr<SwrContext, SwrContextDeleter>;
//     using CodecContextPtr   = std::unique_ptr<AVCodecContext, AVCodecContextDeleter>;
//     using FormatContextPtr  = std::unique_ptr<AVFormatContext, AVFormatContextDeleter>;
// signals:
//     void data_ffmpeg(QSharedPointer<QByteArray> pdata);
//     void error_ffmpeg(const QString& msg);
//     void message_ffmpeg(const QString& msg);
// public:
//     explicit FFmpegAudioTask(QObject* parent = nullptr, AudioTaskBufferType type = AudioTaskBufferType::Output) noexcept : AudioTaskBase(type, parent) {};
//     ~FFmpegAudioTask() {cleanup_data();};

//     virtual void play() override;
//     virtual bool merge(const std::vector<TimeStampPair>& timestamp_list
//         , const std::vector<std::string> &input_file, const QString& output_file
//         , const FFmpegFormatConfig& config, FFmpegMergeOption option) override;
//     virtual bool encode(const FFmpegFormatConfig& config, const QString& output_file);
//     virtual bool decode(const FFmpegFormatConfig& config, bool is_emit) override;
//     virtual bool decode(std::function<void(std::span<const uint8_t>)> f_pcmdata) override;
//     virtual bool transcode(const QString& input_file, const QString& output_file) override;
// protected:
//     virtual bool initialize(const QString& file_path) noexcept override;
// private:
//     void emit_formatted_error(const QString& message);
//     void emit_formatted_error(const char* prefix, const int error_code);
//     void emit_formatted_message(const QString& message);
    
//     void switch_mode() noexcept;
//     bool initialize_output(const FFmpegFormatConfig &config, const QString& file_path) noexcept;
//     bool merge_mixing(const std::vector<TimeStampPair>& timestamp_list, const std::vector<std::string> &input_file, const QString& output_file, const FFmpegFormatConfig& config);

//     QString             _file_path{};
//     SwrContextPtr       _swr_context_i{nullptr};
//     CodecContextPtr     _codec_context_i{nullptr};
//     FormatContextPtr    _format_context_i{nullptr};

//     SwrContextPtr       _swr_context_o{nullptr};
//     CodecContextPtr     _codec_context_o{nullptr};
//     FormatContextPtr    _format_context_o{nullptr};

//     int                 _stream_index{-1};
//     AVRational          _stream_timebase{0,0};
// };

// inline void FFmpegAudioTask::emit_formatted_error(const QString& message)
// {
//     emit error_ffmpeg(message);
// }
// inline void FFmpegAudioTask::emit_formatted_error(const char *prefix, const int error_code)
// {
//     char errbuf[AV_ERROR_MAX_BUFFER_SIZE];
//     av_strerror(error_code, errbuf, sizeof(errbuf));
//     QString message{QString::fromUtf8(prefix)+" with error: "+QString::fromUtf8(errbuf)};
//     emit error_ffmpeg(message);
// }
// inline void FFmpegAudioTask::emit_formatted_message(const QString &message)
// {
//     emit message_ffmpeg(message);
// }
// inline void FFmpegAudioTask::switch_mode() noexcept {
//     if (_role_buffer == AudioTaskBufferType::Receiving) {
//         _role_buffer = AudioTaskBufferType::Output;
//     } else  if (_role_buffer == AudioTaskBufferType::Output) {
//         _role_buffer = AudioTaskBufferType::Receiving;
//     }
// }