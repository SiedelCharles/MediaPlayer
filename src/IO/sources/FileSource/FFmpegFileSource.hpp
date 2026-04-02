#include "FileSource.hpp"
extern "C" {
    #include <libavutil/time.h>
    #include <libavutil/avutil.h>
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswresample/swresample.h>
}
namespace audiotask::source {
/// @name RAII deleter for ffmpeg resources
/// {@
struct AVFrameDeleter {
    void operator()(AVFrame* f){av_frame_free(&f);};
};
struct AVPacketDeleter {
    void operator()(AVPacket* p){av_packet_free(&p);};
};
struct SwrContextDeleter { ///< deleter for smart pointers
    void operator()(SwrContext* ctx) const { swr_free(&ctx); }
};
struct AVCodecContextDeleter { ///< deleter for smart pointers
    void operator()(AVCodecContext* ctx) const { avcodec_free_context(&ctx); }
};
struct AVFormatContextDeleter { ///< deleter for smart pointers
    void operator()(AVFormatContext* ctx) const { avformat_close_input(&ctx); }
};
/// @}
using FramePtr          = std::unique_ptr<AVFrame, AVFrameDeleter>;
using PacketPtr         = std::unique_ptr<AVPacket, AVPacketDeleter>;
using SwrContextPtr     = std::unique_ptr<SwrContext, SwrContextDeleter>;
using CodecContextPtr   = std::unique_ptr<AVCodecContext, AVCodecContextDeleter>;
using FormatContextPtr  = std::unique_ptr<AVFormatContext, AVFormatContextDeleter>;
class FFmpegFileSource : public FileSource  {
private:
    mutable std::mutex _operation_mutex;
    /// @todo supplement error code or status
    int _stream_index{-1};
    SwrContextPtr _swr_context{nullptr};
    CodecContextPtr _codec_context{nullptr};
    FormatContextPtr _format_context{nullptr};

    SampleFormat to_sample_format(AVSampleFormat fmt) noexcept;
    AVSampleFormat to_av_sample_format(SampleFormat fmt) noexcept;
    void reset_ffmpeg_resources() noexcept;
    bool setup_resampler() noexcept;
public:
    virtual void run() override;
    virtual bool init(const std::string& file_path) noexcept override;
    virtual bool seek(double time_sec) noexcept override;
    virtual bool close() noexcept override;
};
}