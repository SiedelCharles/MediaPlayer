#include "FFmpegAudioTask.h"

#include "QtCore/qdebug.h"

bool FFmpegAudioTask::initialize(const QString &file_path)
{
    AVFormatContext *format_context{nullptr};
    if (auto i_result = avformat_open_input(&format_context, file_path.toUtf8().constData(), nullptr, nullptr)
        ; i_result < 0) {
        emit_formatted_error("Can't open the file", i_result);
        return false;
    }
    _format_context.reset(format_context);
    if (auto i_result = avformat_find_stream_info(_format_context.get(), nullptr)
        ; i_result < 0) {
        emit_formatted_error("Can't find the message of file stream", i_result);
        return false;
    }
    /// @todo Refine the logic for stream selection
    if (_stream_index = av_find_best_stream(_format_context.get(), AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0)
        ; _stream_index < 0) {
        emit_formatted_error("Can't find the file stream", _stream_index);
        return false;
    }
    _stream_timebase = _format_context->streams[_stream_index]->time_base;
    AVCodecParameters *codec_params = _format_context->streams[_stream_index]->codecpar;
    const AVCodec *codec = avcodec_find_decoder(codec_params->codec_id);
    if (!codec) {
        emit_formatted_error("Unsupported encoding format");
        return false;
    }
    AVCodecContext *codec_context{avcodec_alloc_context3(codec)};
    if (!codec_context) {
        emit_formatted_error("Failed to allocate codec context");
        return false;
    }
    _codec_context.reset(codec_context);
    if (auto i_result = avcodec_parameters_to_context(_codec_context.get(), codec_params)
    ; i_result < 0) {
        emit_formatted_error("Failed to copy codec parameters", i_result);
        return false;
    }
    if (auto i_result = avcodec_open2(_codec_context.get(), codec, nullptr)
        ; i_result < 0) {
        emit_formatted_error("Failed to open codec", i_result);
        return false;
    }
    _file_path = file_path;
    emit_formatted_message("Initialized successfully");
    return true;
}

void FFmpegAudioTask::decode(const FFmpegFormatConfig& config)
{
    if (auto b_result = is_initialized()
        ; !b_result) {
        emit_formatted_error("uninitialized");
        return ;
    }
    SwrContext *swr_context{nullptr};
    FFmpegFormatConfig destination_config{};
    /// @todo this is error logic. ??
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
                emit_formatted_error("Failed to alloc swr", i_result);
                return ;
            }
            if (auto i_result = swr_init(swr_context)
            ; i_result < 0) {
                emit_formatted_error("Failed to init swr", i_result);
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
        emit_formatted_error("Packet and Frames is null");
        return ;
    }
    std::vector<uint8_t> buffer;
    while(!_atomic_cancel.load(std::memory_order_acquire)) {
        /// @brief process packet
        if (auto i_result = av_read_frame(_format_context.get(), packet.get())
        ; i_result == AVERROR_EOF) {
            break;
        } else if (i_result < 0) {
            emit_formatted_error("av_read_frame", i_result);
            break;
        }
        if (packet->stream_index != _stream_index) {
            av_packet_unref(packet.get());
            continue;
        }
        auto i_result = avcodec_send_packet(_codec_context.get(), packet.get());
        av_packet_unref(packet.get());
        if (i_result < 0) {
            emit_formatted_error("avcodec_send_packet", i_result);
            break;
        }
        /// @brief process frame
        while(true) {
            if (auto i_result = avcodec_receive_frame(_codec_context.get(), frame.get())
            ; i_result == AVERROR(EAGAIN) || i_result == AVERROR_EOF) {
                break;
            } else if (i_result < 0) {
                emit_formatted_error("avcodec_receive_frame", i_result);
                break;
            }
            if (_swr_context) {
                int samples = swr_get_out_samples(_swr_context.get(), frame->nb_samples);
                auto size_buffer = av_samples_get_buffer_size(nullptr, destination_config._channel_count, samples, destination_config._sample_format, 1);
                if (size_buffer < 0) {
                    emit_formatted_error("Failed to get buffer size");
                    av_frame_unref(frame.get());
                    break;
                }
                if (buffer.size() < static_cast<size_t>(size_buffer)) {
                    buffer.resize(static_cast<size_t>(size_buffer));
                }
                uint8_t *transmit_data[] = {buffer.data()};
                auto size_converted = swr_convert(_swr_context.get(), transmit_data, samples,
                                                        (const uint8_t**)frame->data, frame->nb_samples); 
                if (size_converted < 0) {
                    emit_formatted_message("size_converted is less than 0");
                    av_frame_unref(frame.get());
                    break;
                } else if (size_converted == 0) {
                    av_frame_unref(frame.get());
                    continue;
                }
                auto size_transmit = av_samples_get_buffer_size(nullptr, destination_config._channel_count, size_converted, destination_config._sample_format, 1);
                if (size_transmit > 0) {
                    auto sp_data = QSharedPointer<QByteArray>::create(reinterpret_cast<const char*>(buffer.data()), size_transmit);
                    emit data_ffmpeg(sp_data);
                }
            } else {
                /// @brief no need for resample, emit data directly
                /// @todo process planar data
                auto size_transmit = frame->nb_samples * _codec_context->ch_layout.nb_channels * av_get_bytes_per_sample(_codec_context->sample_fmt);
                if (size_transmit < 0) {
                    emit_formatted_message("size_transmit is less than 0");
                    av_frame_unref(frame.get());
                    break;
                }
                auto sp_data = QSharedPointer<QByteArray>::create(reinterpret_cast<const char*>(frame->data[0]), size_transmit);
                emit data_ffmpeg(sp_data);
            }
        }
        av_frame_unref(frame.get());
    }
    /// @todo process remain frames
    auto i_result = avcodec_send_packet(_codec_context.get(), nullptr);
    while(true) {
        if (auto i_result = avcodec_receive_frame(_codec_context.get(), frame.get())
        ; i_result == AVERROR(EAGAIN) || i_result == AVERROR_EOF) {
            break;
        } else if (i_result < 0) {
            emit_formatted_error("avcodec_receive_frame", i_result);
            break;
        }
        if (_swr_context) {
            int samples = swr_get_out_samples(_swr_context.get(), frame->nb_samples);
            auto size_buffer = av_samples_get_buffer_size(nullptr, destination_config._channel_count, samples, destination_config._sample_format, 1);
            if (size_buffer < 0) {
                emit_formatted_error("Failed to get buffer size");
                av_frame_unref(frame.get());
                break;
            }
            if (buffer.size() < static_cast<size_t>(size_buffer)) {
                buffer.resize(static_cast<size_t>(size_buffer));
            }
            uint8_t *transmit_data[] = {buffer.data()};
            auto size_converted = swr_convert(_swr_context.get(), transmit_data, samples,
                                                    (const uint8_t**)frame->data, frame->nb_samples); 
            if (size_converted < 0) {
                emit_formatted_message("size_converted is less than 0");
                av_frame_unref(frame.get());
                break;
            } else if (size_converted == 0) {
                av_frame_unref(frame.get());
                continue;
            }
            auto size_transmit = av_samples_get_buffer_size(nullptr, destination_config._channel_count, size_converted, destination_config._sample_format, 1);
            if (size_transmit > 0) {
                auto sp_data = QSharedPointer<QByteArray>::create(reinterpret_cast<const char*>(buffer.data()), size_transmit);
                emit data_ffmpeg(sp_data);
            }
        } else {
            /// @brief no need for resample, emit data directly
            /// @todo process planar data
            auto size_transmit = frame->nb_samples * _codec_context->ch_layout.nb_channels * av_get_bytes_per_sample(_codec_context->sample_fmt);
            if (size_transmit < 0) {
                emit_formatted_message("size_transmit is less than 0");
                av_frame_unref(frame.get());
                break;
            }
            auto sp_data = QSharedPointer<QByteArray>::create(reinterpret_cast<const char*>(frame->data[0]), size_transmit);
            emit data_ffmpeg(sp_data);
        }
        av_frame_unref(frame.get());
    }
    /// @todo flush swr_context
    /// @todo emit finish signal;
    emit message_finished();
    return ;
}

void FFmpegAudioTask::decode(std::function<void(std::span<const uint8_t>)> f_pcmdata)
{
    return ;
}

void FFmpegAudioTask::transcode(const QStringList &input_file, const QString &output_file)
{
    return ;
}

void FFmpegAudioTask::play()
{
    return ;
}

void FFmpegAudioTask::merge(const QStringList &input_file, const QString &output_file, Merge_Option option)
{
    switch(option) {
        case Merge_Option::MergeConcatenate:
            break;
        case Merge_Option::MergeInsert:
            break;
        case Merge_Option::MergeMixing:
            break;
    }
}

void FFmpegAudioTask::encode(const FFmpegFormatConfig &config)
{
    return ;
}
