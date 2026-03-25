#pragma once

#include "AudioTaskInterface.h"
#include <span>
#include <atomic>
#include <cstdint>
#include <functional>

extern "C" {
    #include <libavutil/time.h>
    #include <libavutil/avutil.h>
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswresample/swresample.h>
}

/// @brief deleter for smart pointers
struct SwrContextDeleter {
    void operator()(SwrContext* ctx) const { swr_free(&ctx); }
};
/// @brief deleter for smart pointers
struct AVCodecContextDeleter {
    void operator()(AVCodecContext* ctx) const { avcodec_free_context(&ctx); }
};
/// @brief deleter for smart pointers
struct AVFormatContextDeleter {
    void operator()(AVFormatContext* ctx) const { avformat_close_input(&ctx); }
};

/// @brief to register
static bool bool_meta_type_register = []() {
    qRegisterMetaType<QSharedPointer<QByteArray>>();
    return true;
}(); /// @brief Add "()" is to solve a known issue in the MSVC compiler regarding the implicit conversion of non-capturing lambdas to bool

/// @brief audio resampling format
struct FFmpegFormatConfig{
    int _sample_rate = 0;
    int _channel_count = 0;
    AVSampleFormat _sample_format = AV_SAMPLE_FMT_NONE;
    FFmpegFormatConfig() = default;
    FFmpegFormatConfig(int sample_rate, int channel_count, AVSampleFormat sample_format):_sample_rate(sample_rate), _channel_count(channel_count), _sample_format(sample_format){}
    FFmpegFormatConfig(const FFmpegFormatConfig&) = default;
    FFmpegFormatConfig(FFmpegFormatConfig&&) noexcept=default;
    FFmpegFormatConfig& operator=(const FFmpegFormatConfig&)=default;
    FFmpegFormatConfig& operator=(FFmpegFormatConfig&&) noexcept=default;
    bool operator==(const FFmpegFormatConfig& other) const {
        return  _sample_rate == other._sample_rate &&
                _channel_count == other._channel_count &&
                _sample_format == other._sample_format;
    }
    bool operator!=(const FFmpegFormatConfig& other) const {
        return !(*this==other);
    }
};

class FFmpegAudioTask : public AudioTask
{
    Q_OBJECT
    using SwrContextPtr     = std::unique_ptr<SwrContext, SwrContextDeleter>;
    using CodecContextPtr   = std::unique_ptr<AVCodecContext, AVCodecContextDeleter>;
    using FormatContextPtr  = std::unique_ptr<AVFormatContext, AVFormatContextDeleter>;
signals:
    void data_ffmpeg(QSharedPointer<QByteArray> pdata);
    void error_ffmpeg(const QString& msg);
public:
    explicit FFmpegAudioTask(QObject* parent = nullptr) noexcept : AudioTask(parent){}
    // virtual void cancle() noexcept override;
    virtual void play() override;
    virtual void merge(const QStringList& input_file, const QString& output_file, Merge_Option option) override;
    virtual void encode(const FFmpegFormatConfig& config) override;
    virtual void decode(const FFmpegFormatConfig& config) override;
    virtual void decode(std::function<void(std::span<const uint8_t>)> f_pcmdata) override;
    virtual void transcode(const QStringList& input_file, const QString& output_file) override;
protected:
    virtual bool initialize(const QString& file_path) override;
private:
    void emit_formatted_error(const QString& message);
    void emit_formatted_error(const char* prefix, const int error_code);
    void emit_formatted_message(const QString& message);

    QString             _file_path{};
    SwrContextPtr       _swr_context{nullptr};
    CodecContextPtr     _codec_context{nullptr};
    FormatContextPtr    _format_context{nullptr};

    int                 _stream_index{-1};
    AVRational          _stream_timebase{0,0};
};

inline void FFmpegAudioTask::emit_formatted_error(const QString& message)
{
    emit error_ffmpeg(message);
}
inline void FFmpegAudioTask::emit_formatted_error(const char *prefix, const int error_code)
{
    char errbuf[AV_ERROR_MAX_BUFFER_SIZE];
    av_strerror(error_code, errbuf, sizeof(errbuf));
    QString message{QString::fromUtf8(prefix)+" with error: "+QString::fromUtf8(errbuf)};
    emit error_ffmpeg(message);
}
inline void FFmpegAudioTask::emit_formatted_message(const QString &message)
{
    emit message_ffmpeg(message);
}
