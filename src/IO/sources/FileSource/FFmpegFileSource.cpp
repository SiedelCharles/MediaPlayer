#include "FFmpegFileSource.hpp"
namespace audiotask::source {
SampleFormat audiotask::source::FFmpegFileSource::to_sample_format(AVSampleFormat fmt) noexcept
{
    switch (fmt) {
        case AV_SAMPLE_FMT_S16:
        case AV_SAMPLE_FMT_S16P:
            return SampleFormat::S16;
        case AV_SAMPLE_FMT_S32:
        case AV_SAMPLE_FMT_S32P:
            return SampleFormat::S32;
        case AV_SAMPLE_FMT_FLT:
        case AV_SAMPLE_FMT_FLTP:
            return SampleFormat::F32;
        case AV_SAMPLE_FMT_DBL:
        case AV_SAMPLE_FMT_DBLP:
            return SampleFormat::F64;
        default:
            return SampleFormat::Undefined;
    }
}
AVSampleFormat FFmpegFileSource::to_av_sample_format(SampleFormat fmt) noexcept {
    switch (fmt) {
        case SampleFormat::S16: return AV_SAMPLE_FMT_S16;
        case SampleFormat::S32: return AV_SAMPLE_FMT_S32;
        case SampleFormat::F32: return AV_SAMPLE_FMT_FLT;
        case SampleFormat::F64: return AV_SAMPLE_FMT_DBL;
        default:                return AV_SAMPLE_FMT_NONE;
    }
}
void FFmpegFileSource::reset_ffmpeg_resources() noexcept {
    _swr_context.reset();
    _codec_context.reset();
    _format_context.reset();
    _stream_index = -1;
}
bool FFmpegFileSource::setup_resampler() noexcept {
    if (!_codec_context) return false;
    SourceFormat out_fmt{};
    {
        std::lock_guard<std::mutex> lock(_state_mutex);
        out_fmt = _output_format;
    }
    if (out_fmt.channels <= 0 ||
        out_fmt.sample_rate <= 0 ||
        out_fmt.format == SampleFormat::Undefined) {
        return false;
    }
    /// @todo This may be redundant here.
    const AVSampleFormat dst_fmt = to_av_sample_format(out_fmt.format);
    if (dst_fmt == AV_SAMPLE_FMT_NONE) return false;
    AVChannelLayout out_layout{};
    av_channel_layout_default(&out_layout, out_fmt.channels);
    if (out_layout.nb_channels <= 0) {
        return false;
    }
    SwrContext* swr = nullptr;
    const int ret = swr_alloc_set_opts2(
        &swr,
        &out_layout,
        dst_fmt,
        out_fmt.sample_rate,
        &_codec_context->ch_layout,
        _codec_context->sample_fmt,
        _codec_context->sample_rate,
        0,
        nullptr
    );
    av_channel_layout_uninit(&out_layout);
    if (ret < 0 or not swr) {
        if (swr) swr_free(&swr);
        return false;
    }
    if (swr_init(swr) < 0) {
        swr_free(&swr);
        return false;
    }
    _swr_context.reset(swr);
    return true;
}
bool FFmpegFileSource::init(const std::string& file_path) noexcept {
    std::lock_guard<std::mutex> op_lock(_operation_mutex);
    if (_running.load()) return false;
    if (_initialized.exchange(true)) return false;

    AVFormatContext *format_context{nullptr};
    if (auto i_result = avformat_open_input(&format_context, file_path.data(), nullptr, nullptr)
        ; i_result < 0) {
        _initialized.store(false);
        return false;
    }
    _format_context.reset(format_context);
    if (auto i_result = avformat_find_stream_info(_format_context.get(), nullptr)
        ; i_result < 0) {
        _initialized.store(false);
        return false;
    }
    /// @todo Refine the logic for stream selection
    _stream_index = av_find_best_stream(_format_context.get(), AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (_stream_index < 0) {
        _initialized.store(false);
        return false;
    }
    /// @todo _stream_timebase = _format_context->streams[_stream_index]->time_base;
    AVStream* stream = _format_context->streams[_stream_index];
    if (!stream) {
        reset_ffmpeg_resources();
        _initialized.store(false);
        return false;
    }
    AVCodecParameters* codec_params = stream->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codec_params->codec_id);
    if (!codec) {
        reset_ffmpeg_resources();
        _initialized.store(false);
        return false;
    }
    AVCodecContext* codec_context = avcodec_alloc_context3(codec);
    if (!codec_context) {
        reset_ffmpeg_resources();
        _initialized.store(false);
        return false;
    }
    _codec_context.reset(codec_context);
    if (avcodec_parameters_to_context(_codec_context.get(), codec_params) < 0) {
        reset_ffmpeg_resources();
        _initialized.store(false);
        return false;
    }
    if (avcodec_open2(_codec_context.get(), codec, nullptr) < 0) {
        reset_ffmpeg_resources();
        _initialized.store(false);
        return false;
    }
    SourceFormat detected{};
    detected.channels = _codec_context->ch_layout.nb_channels;
    detected.sample_rate = _codec_context->sample_rate;
    detected.duration = _format_context->duration > 0
        ? static_cast<double>(_format_context->duration) / static_cast<double>(AV_TIME_BASE)
        : 0.0;
    detected.format = to_sample_format(_codec_context->sample_fmt);
    {
        std::lock_guard<std::mutex> state_lock(_state_mutex);
        _file_path = file_path;
        _source_format = detected;
        if (_output_format.channels <= 0 ||
            _output_format.sample_rate <= 0 ||
            _output_format.format == SampleFormat::Undefined) {
            _output_format = detected;
        }
    }
    if (!setup_resampler()) {
        reset_ffmpeg_resources();
        _initialized.store(false);
        return false;
    }
    return true;
}

bool FFmpegFileSource::seek(double time_sec) noexcept {
    std::lock_guard<std::mutex> op_lock(_operation_mutex);

    if (!_initialized.load()) return false;
    if (!_format_context || !_codec_context || _stream_index < 0) return false;
    if (time_sec < 0.0) return false;

    AVStream* stream = _format_context->streams[_stream_index];
    if (!stream) return false;

    const int64_t target_ts = av_rescale_q(
        static_cast<int64_t>(time_sec * AV_TIME_BASE),
        AVRational{1, AV_TIME_BASE},
        stream->time_base
    );

    if (av_seek_frame(_format_context.get(), _stream_index, target_ts, AVSEEK_FLAG_BACKWARD) < 0) {
        return false;
    }

    avcodec_flush_buffers(_codec_context.get());

    if (_swr_context) {
        swr_close(_swr_context.get());
        if (swr_init(_swr_context.get()) < 0) {
            return false;
        }
    }

    return true;
}

void FFmpegFileSource::run() {
    FramePtr frame(av_frame_alloc());
    PacketPtr packet(av_packet_alloc());
    if (!frame || !packet) {
        _running.store(false);
        return;
    }
    while (_running.load()) {
        int read_result = 0;
        {
            std::lock_guard<std::mutex> op_lock(_operation_mutex);
            if (!_initialized.load() ||
                !_format_context ||
                !_codec_context ||
                !_swr_context ||
                _stream_index < 0) {
                break;
            }
            read_result = av_read_frame(_format_context.get(), packet.get());
        }
        if (read_result < 0) {
            break;
        }
        if (packet->stream_index != _stream_index) {
            av_packet_unref(packet.get());
            continue;
        }
        {
            std::lock_guard<std::mutex> op_lock(_operation_mutex);
            if (auto b_result = avcodec_send_packet(_codec_context.get(), packet.get())
            ; b_result < 0) {
                av_packet_unref(packet.get());
                continue;
            }
        }
        av_packet_unref(packet.get());
        while (_running.load()) {
            int recv_result = 0;
            {
                std::lock_guard<std::mutex> op_lock(_operation_mutex);
                recv_result = avcodec_receive_frame(_codec_context.get(), frame.get());
            }
            if (recv_result == AVERROR(EAGAIN) || recv_result == AVERROR_EOF) {
                break;
            }
            if (recv_result < 0) {
                break;
            }
            SourceFormat out_fmt{};
            {
                std::lock_guard<std::mutex> state_lock(_state_mutex);
                out_fmt = _output_format;
            }
            const AVSampleFormat dst_fmt = to_av_sample_format(out_fmt.format);
            if (dst_fmt == AV_SAMPLE_FMT_NONE ||
                out_fmt.channels <= 0 ||
                out_fmt.sample_rate <= 0) {
                av_frame_unref(frame.get());
                continue;
            }
            const int dst_nb_samples = av_rescale_rnd(
                swr_get_delay(_swr_context.get(), _codec_context->sample_rate) + frame->nb_samples,
                out_fmt.sample_rate,
                _codec_context->sample_rate,
                AV_ROUND_UP
            );
            uint8_t** dst_data = nullptr;
            int dst_linesize = 0;
            if (av_samples_alloc_array_and_samples(
                    &dst_data,
                    &dst_linesize,
                    out_fmt.channels,
                    dst_nb_samples,
                    dst_fmt,
                    0) < 0) {
                av_frame_unref(frame.get());
                continue;
            }
            int converted_samples = 0;
            {
                std::lock_guard<std::mutex> op_lock(_operation_mutex);
                converted_samples = swr_convert(
                    _swr_context.get(),
                    dst_data,
                    dst_nb_samples,
                    const_cast<const uint8_t**>(frame->extended_data),
                    frame->nb_samples
                );
            }
            if (converted_samples > 0) {
                const int buffer_size = av_samples_get_buffer_size(
                    &dst_linesize,
                    out_fmt.channels,
                    converted_samples,
                    dst_fmt,
                    1
                );
                if (buffer_size > 0) {
                    auto pad = get_pad(core::Direction::Sending);
                    if (pad) {
                        AVBufferRef* ref = av_buffer_alloc(static_cast<size_t>(buffer_size));
                        if (ref && ref->data) {
                            /// @todo optimize here
                            std::memcpy(ref->data, dst_data[0], static_cast<size_t>(buffer_size));
                            try {
                                core::AudioTaskBufferList buffer_list(ref);
                                ref = nullptr;
                                const auto flow = pad->push(std::move(buffer_list));
                                if (flow == core::FlowReturn::Ended ||
                                    flow == core::FlowReturn::Failing ||
                                    flow == core::FlowReturn::Flushing) {
                                    if (dst_data) {
                                        av_freep(&dst_data[0]);
                                        av_freep(&dst_data);
                                    }
                                    av_frame_unref(frame.get());
                                    _running.store(false);
                                    return;
                                }
                            } catch (...) {
                                if (ref) av_buffer_unref(&ref);
                                if (dst_data) {
                                    av_freep(&dst_data[0]);
                                    av_freep(&dst_data);
                                }
                                av_frame_unref(frame.get());
                                _running.store(false);
                                return;
                            }
                        } else {
                            if (ref) av_buffer_unref(&ref);
                        }
                    }
                }
            }
            if (dst_data) {
                av_freep(&dst_data[0]);
                av_freep(&dst_data);
            }
            av_frame_unref(frame.get());
        }
    }
    _running.store(false);
}
bool FFmpegFileSource::close() noexcept {
    stop();
    std::lock_guard<std::mutex> op_lock(_operation_mutex);
    if (!_initialized.exchange(false)) return false;
    reset_ffmpeg_resources();
    {
        std::lock_guard<std::mutex> state_lock(_state_mutex);
        _file_path.clear();
        _source_format = {};
        _output_format = {};
    }
    return true;
}
}