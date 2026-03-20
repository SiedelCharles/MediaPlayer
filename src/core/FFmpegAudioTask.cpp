#include "FFmpegAudioTask.h"


void FFmpegAudioTask::cancle() noexcept
{
    _atomic_cancle.store(true, std::memory_order_acquire);
}

bool FFmpegAudioTask::initialize(const QString &file_path)
{
    AVFormatContext *format_context{nullptr};
    if (auto i_result = avformat_open_input(&format_context, file_path.toUtf8().constData(), nullptr, nullptr)
        ; i_result < 0) {
        emit_formatted_error("", i_result);
        return false;
    }
    _format_context.reset(format_context);
    if (auto i_result = avformat_find_stream_info(_format_context.get(), nullptr)
        ; i_result < 0) {
        emit_formatted_error("", i_result);
        return false;
    }
    /// @todo Refine the logic for stream selection
    if (_stream_index = av_find_best_stream(_format_context.get(), AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0)
        ; _stream_index < 0) {
        emit_formatted_error("", _stream_index);
        return false;
    }
    _stream_timebase = _format_context->streams[_stream_index]->time_base;
    AVCodecParameters *codec_params = _format_context->streams[_stream_index]->codecpar;
    const AVCodec *codec = avcodec_find_decoder(codec_params->codec_id);
    if (!codec) {
        /// @todo emit error without error code;
        return false;
    }
    AVCodecContext *codec_context{avcodec_alloc_context3(codec)};
    if (!_codec_context) {
        /// @todo emit error without error code;
        return false;
    }
    _codec_context.reset(codec_context);
    if (auto i_result = avcodec_parameters_to_context(_codec_context.get(), codec_params)
    ; i_result < 0) {
        emit_formatted_error("", i_result);
        return false;
    }
    if (auto i_result = avcodec_open2(_codec_context.get(), codec, nullptr)
        ; i_result < 0) {
        emit_formatted_error("", i_result);
        return false;
    }
    _file_path = file_path;
    return true;
}


void FFmpegAudioTask::decode(const FFmpegFormatConfig& config)
{
    if (auto b_result = is_initialized()
        ; !b_result) {
        /// @todo emit error signal
        return ;
    }
    SwrContext *swr_context{nullptr};
    FFmpegFormatConfig destination_config{};
    /// @todo this is error logic
    if (auto b_result = config == destination_config
        ; b_result) {
            /// \brief no need for resample
            _swr_context.reset();
        } else {
            AVChannelLayout destination_layout;
            if (config._channel_count) {
                av_channel_layout_default(&destination_layout, config._channel_count);
            } else {
                destination_layout = _codec_context->ch_layout;
                destination_config._channel_count = destination_layout.nb_channels;
            }
            destination_config._sample_rate = config._sample_rate == destination_config._sample_rate ? _codec_context->sample_rate : config._sample_rate;
            destination_config._sample_format = config._sample_format == destination_config._sample_format ? _codec_context->sample_fmt : config._sample_format;
            
            if (auto i_result = swr_alloc_set_opts2(&swr_context,
                &destination_layout, destination_config._sample_format, destination_config._sample_rate, 
                &_codec_context->ch_layout, _codec_context->sample_fmt, _codec_context->sample_rate,
                0, nullptr)
                ; i_result < 0) {
                /// @todo emit error signal
                emit_formatted_error("", i_result);
                return ;
            }
        }
    _swr_context.reset(swr_context);
    /// @details Warning: The pointer nullified by av_frame_free is a temporary object,
    /// not the pointer managed by the unique_ptr.
    /// However, the unique_ptr itself will nullify its own pointer afterward,
    /// so this has little impact. Unless other code during the deleter's execution 
    /// relies on the pointer still pointing to a valid object,
    /// nullifying the original pointer early has no effect.
    auto FrameDeleter = [](AVFrame* f){av_frame_free(&f);};
    auto PacketDeleter = [](AVPacket* p){av_packet_free(&p);};
    std::unique_ptr<AVFrame, decltype(FrameDeleter)> frame{av_frame_alloc(), FrameDeleter};
    std::unique_ptr<AVPacket, decltype(PacketDeleter)> packet{av_packet_alloc(), PacketDeleter};
    if(!frame || !packet) {
        /// @todo emit error signal
        return ;
    }
    std::vector<uint8_t> buffer;
    while(!_atomic_cancle.load(std::memory_order_acquire)) {
        /// @brief process packet
        if (auto i_result = av_read_frame(_format_context.get(), packet.get())
        ; i_result < 0) {
            emit_formatted_error("", i_result);
            break;
        }
        if (packet->stream_index != _stream_index) {
            av_packet_unref(packet.get());
            continue;
        }
        auto i_result = avcodec_send_packet(_codec_context.get(), packet.get());
        av_packet_unref(packet.get());
        if (i_result < 0) {
            emit_formatted_error("", i_result);
            break;
        }
        /// @brief process frame
        while(true) {
            if (auto i_result = avcodec_receive_frame(_codec_context.get(), frame.get())
            ; i_result == AVERROR(EAGAIN) || i_result == AVERROR_EOF) {
                break;
            } else if (i_result < 0) {
                /// @todo emit error signal
                emit_formatted_error("", i_result);
                break;
            }
            if(_swr_context) {
                int samples = swr_get_out_samples(_swr_context.get(), frame->nb_samples);
                auto size_buffer = av_samples_get_buffer_size(nullptr, destination_config._channel_count, samples, destination_config._sample_format, 1);
                if (size_buffer < 0) {
                    /// @todo emit error signal
                    break;
                }
                if (buffer.size() < static_cast<size_t>(size_buffer)) {
                    buffer.resize(static_cast<size_t>(size_buffer));
                }
                uint8_t *transmit_data[] = {buffer.data()};
                auto size_converted = swr_convert(_swr_context.get(), transmit_data, samples,
                                                        (const uint8_t**)frame->data, frame->nb_samples); 
                if (size_converted < 0) {
                    /// @todo emit error signal
                    av_frame_unref(frame.get());
                    break;
                } else if (size_converted == 0) {
                    continue;
                }
                auto size_transmit = av_samples_get_buffer_size(nullptr, destination_config._channel_count, size_converted, destination_config._sample_format, 1);
                if(size_transmit > 0) {
                    auto sp_data = QSharedPointer<QByteArray>::create(reinterpret_cast<const char*>(buffer.data()), size_transmit);
                    /// @todo emit data

                }
            } else {
                /// @brief no need for resample, emit data directly
                /// @todo
            }
            
        }
    }
    /// @todo process remain frames
}

void FFmpegAudioTask::decode(std::function<void(std::span<const uint8_t>)> f_pcmdata)
{
    return ;
}