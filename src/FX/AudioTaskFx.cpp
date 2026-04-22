#include "FFmpegAudioTask.hpp"
// #include <iostream>
// #include "QtCore/qdebug.h"
#include <iostream>

constexpr double merge_gain = 0.15;

bool FFmpegAudioTask::initialize(const QString &file_path) noexcept
{
    AVFormatContext *format_context{nullptr};
    if (auto i_result = avformat_open_input(&format_context, file_path.toUtf8().constData(), nullptr, nullptr)
        ; i_result < 0) {
        emit_formatted_error("Can't open the file", i_result);
        return false;
    }
    _format_context_i.reset(format_context);
    if (auto i_result = avformat_find_stream_info(_format_context_i.get(), nullptr)
        ; i_result < 0) {
        emit_formatted_error("Can't find the message of file stream", i_result);
        return false;
    }
    /// @todo Refine the logic for stream selection
    if (_stream_index = av_find_best_stream(_format_context_i.get(), AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0)
        ; _stream_index < 0) {
        emit_formatted_error("Can't find the file stream", _stream_index);
        return false;
    }
    _stream_timebase = _format_context_i->streams[_stream_index]->time_base;
    AVCodecParameters *codec_params = _format_context_i->streams[_stream_index]->codecpar;
    /// @todo this could be reuse
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
    _codec_context_i.reset(codec_context);
    if (auto i_result = avcodec_parameters_to_context(_codec_context_i.get(), codec_params)
    ; i_result < 0) {
        emit_formatted_error("Failed to copy codec parameters", i_result);
        return false;
    }
    if (auto i_result = avcodec_open2(_codec_context_i.get(), codec, nullptr)
        ; i_result < 0) {
        emit_formatted_error("Failed to open codec", i_result);
        return false;
    }
    _file_path = file_path;
    emit_formatted_message("Initialized successfully");
    return true;
}

bool FFmpegAudioTask::initialize_output(const FFmpegFormatConfig &config, const QString &file_path) noexcept
{
    AVFormatContext *format_context{nullptr};
    if (auto i_result = avformat_alloc_output_context2(&format_context, nullptr, nullptr, file_path.toUtf8().constData())
        ; i_result < 0) {
        emit_formatted_error("Failed to create output context", i_result);
        return false;
    }
    _format_context_o.reset(format_context);

    const AVCodec *codec{avcodec_find_encoder_by_name(config._codec_name.data())};
    if (!codec) {
        emit_formatted_error("Unsupported encoding format");
        return false;
    }

    AVStream* stream{avformat_new_stream(_format_context_o.get(), codec)};
    if (!stream) {
        emit_formatted_error("Failed to create stream");
        return false;
    }
    stream->id = _format_context_o->nb_streams - 1;
    
    AVCodecContext *codec_context{avcodec_alloc_context3(codec)};
    if (!codec_context) {
        emit_formatted_error("Failed to allocate codec context");
        return false;
    }
    _codec_context_o.reset(codec_context);

    _codec_context_o->sample_rate = config._sample_rate;
    av_channel_layout_default(&_codec_context_o->ch_layout, config._channel_count);
    /// @brief in case there is no sample_format
    _codec_context_o->sample_fmt = codec->sample_fmts ? codec->sample_fmts[0] : AV_SAMPLE_FMT_S16;
    /// @todo to set bitrate in config
    // _codec_context_o->bit_rate = config._sample_rate;
    _codec_context_o->time_base = {1, config._sample_rate};

    if (format_context->oformat->flags & AVFMT_GLOBALHEADER) {
        _codec_context_o->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (auto i_result = avcodec_open2(_codec_context_o.get(), codec, nullptr)
        ; i_result < 0) {
        emit_formatted_error("Failed to open codec", i_result);
        return false;
    }
    if (auto i_result = avcodec_parameters_from_context(stream->codecpar, _codec_context_o.get())
    ; i_result < 0) {
        emit_formatted_error("Failed to copy codec parameters", i_result);
        return false;
    }
    stream->time_base = _codec_context_o->time_base;

    if (!(_format_context_o->oformat->flags & AVFMT_NOFILE)) {
        if( auto i_reslut = avio_open(&_format_context_o->pb, file_path.toUtf8().constData(), AVIO_FLAG_WRITE)
        ; i_reslut < 0) {
            emit_formatted_error("Failed to open output file", i_reslut);
            return false;
        }
    }

    return true;
}

bool FFmpegAudioTask::decode(const FFmpegFormatConfig& config, bool is_emit)
{
    if (auto b_result = is_initialized()
        ; !b_result) {
        emit_formatted_error("uninitialized");
        return false;
    }
    if (is_eof() || _role_buffer == AudioTaskBufferType::Input) {
        emit_formatted_error("buffer unavailable.");
        return false;
    }
    SwrContext *swr_context{nullptr};
    FFmpegFormatConfig destination_config{};
    if (auto b_result = (config == destination_config)
        ; b_result) {
            /// @brief no need for resample
            _swr_context_i.reset();
        } else {
            AVChannelLayout destination_layout;
            if (config._channel_count) {
                av_channel_layout_default(&destination_layout, config._channel_count);
            } else {
                destination_layout = _codec_context_i->ch_layout;
            }
            destination_config._channel_count = destination_layout.nb_channels;
            destination_config._sample_rate = config._sample_rate == destination_config._sample_rate ? _codec_context_i->sample_rate : config._sample_rate;
            destination_config._sample_format = config._sample_format == destination_config._sample_format ? _codec_context_i->sample_fmt : config._sample_format;
            if (auto i_result = swr_alloc_set_opts2(&swr_context,
                &destination_layout, destination_config._sample_format, destination_config._sample_rate, 
                &_codec_context_i->ch_layout, _codec_context_i->sample_fmt, _codec_context_i->sample_rate,
                0, nullptr)
                ; i_result < 0) {
                emit_formatted_error("Failed to alloc swr", i_result);
                return false;
            }
            if (auto i_result = swr_init(swr_context)
            ; i_result < 0) {
                emit_formatted_error("Failed to init swr", i_result);
                return false;
            }
        }
    _swr_context_i.reset(swr_context);
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
        return false;
    }
    std::vector<uint8_t> buffer;
    while(!_atomic_cancel.load(std::memory_order_acquire)) {
        /// @brief process packet
        if (auto i_result = av_read_frame(_format_context_i.get(), packet.get())
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
        auto i_result = avcodec_send_packet(_codec_context_i.get(), packet.get());
        av_packet_unref(packet.get());
        if (i_result < 0) {
            emit_formatted_error("avcodec_send_packet", i_result);
            break;
        }
        /// @brief process frame
        while(true) {
            if (auto i_result = avcodec_receive_frame(_codec_context_i.get(), frame.get())
            ; i_result == AVERROR(EAGAIN) || i_result == AVERROR_EOF) {
                break;
            } else if (i_result < 0) {
                emit_formatted_error("avcodec_receive_frame", i_result);
                break;
            }
            if (_swr_context_i) {
                int samples = swr_get_out_samples(_swr_context_i.get(), frame->nb_samples);
                auto size_buffer = av_samples_get_buffer_size(nullptr, destination_config._channel_count, samples, destination_config._sample_format, 1);
                if (size_buffer < 0) {
                    // emit_formatted_error("Failed to get buffer size parse1");
                    av_frame_unref(frame.get());
                    break;
                }
                if (buffer.size() < static_cast<size_t>(size_buffer)) {
                    buffer.resize(static_cast<size_t>(size_buffer));
                }
                uint8_t *transmit_data[] = {buffer.data()};
                auto size_converted = swr_convert(_swr_context_i.get(), transmit_data, samples,
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
                    if(is_emit) {
                        emit data_ffmpeg(sp_data);
                    } else {
                        append_data(sp_data);
                    }
                }
            } else {
                /// @brief no need for resample, emit data directly
                /// @todo process planar data
                auto size_transmit = frame->nb_samples * _codec_context_i->ch_layout.nb_channels * av_get_bytes_per_sample(_codec_context_i->sample_fmt);
                if (size_transmit < 0) {
                    emit_formatted_message("size_transmit is less than 0");
                    av_frame_unref(frame.get());
                    break;
                }
                auto sp_data = QSharedPointer<QByteArray>::create(reinterpret_cast<const char*>(frame->data[0]), size_transmit);
                if(is_emit) {
                    emit data_ffmpeg(sp_data);
                } else {
                    append_data(sp_data);
                }
            }
        }
        av_frame_unref(frame.get());
    }
    /// @todo process remain frames
    auto i_result = avcodec_send_packet(_codec_context_i.get(), nullptr);
    while(true) {
        if (auto i_result = avcodec_receive_frame(_codec_context_i.get(), frame.get())
        ; i_result == AVERROR(EAGAIN) || i_result == AVERROR_EOF) {
            break;
        } else if (i_result < 0) {
            emit_formatted_error("avcodec_receive_frame", i_result);
            break;
        }
        if (_swr_context_i) {
            int samples = swr_get_out_samples(_swr_context_i.get(), frame->nb_samples);
            auto size_buffer = av_samples_get_buffer_size(nullptr, destination_config._channel_count, samples, destination_config._sample_format, 1);
            if (size_buffer < 0) {
                emit_formatted_error("Failed to get buffer size parse2");
                av_frame_unref(frame.get());
                break;
            }
            if (buffer.size() < static_cast<size_t>(size_buffer)) {
                buffer.resize(static_cast<size_t>(size_buffer));
            }
            uint8_t *transmit_data[] = {buffer.data()};
            auto size_converted = swr_convert(_swr_context_i.get(), transmit_data, samples,
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
                if(is_emit) {
                    emit data_ffmpeg(sp_data);
                } else {
                    append_data(sp_data);
                }
            }
        } else {
            /// @brief no need for resample, emit data directly
            /// @todo process planar data
            auto size_transmit = frame->nb_samples * _codec_context_i->ch_layout.nb_channels * av_get_bytes_per_sample(_codec_context_i->sample_fmt);
            if (size_transmit < 0) {
                emit_formatted_message("size_transmit is less than 0");
                av_frame_unref(frame.get());
                break;
            }
            auto sp_data = QSharedPointer<QByteArray>::create(reinterpret_cast<const char*>(frame->data[0]), size_transmit);
            if(is_emit) {
                emit data_ffmpeg(sp_data);
            } else {
                append_data(sp_data);
            }
        }
        av_frame_unref(frame.get());
    }
    /// @todo flush swr_context
    /// @todo emit finish signal;
    // if(!is_emit) {
    //     switch_mode();
    //     /// @todo notice here
    //     eof();
    // }
    emit message_finished();
    return true;
}

bool FFmpegAudioTask::decode(std::function<void(std::span<const uint8_t>)> f_pcmdata)
{
    /// @todo callback function
    return false;
}

bool FFmpegAudioTask::transcode(const QString& input_file, const QString& output_file)
{
    /// decode
    /// encode
    return false;
}

void FFmpegAudioTask::play()
{
    return ;
}

bool FFmpegAudioTask::merge(const std::vector<TimeStampPair> &timestamp_list, const std::vector<std::string> &input_file, const QString &output_file, const FFmpegFormatConfig& config, FFmpegMergeOption option)
{
    bool b_result{false};
    switch(option) {
        case FFmpegMergeOption::MergeConcatenate:
            break;
        case FFmpegMergeOption::MergeInsert:
            break;
        case FFmpegMergeOption::MergeMixing:
            b_result = merge_mixing(timestamp_list, input_file, output_file, config);
            break;
    }
    return b_result;
}

bool FFmpegAudioTask::encode(const FFmpegFormatConfig &config, const QString& output_file)
{
    std::cout << "encode called with output file: " << output_file.toStdString() << std::endl;
    if (auto b_result = initialize_output(config, output_file)
        ; !b_result) {
        emit_formatted_error("Output file failed to initialized");
        return false;
    }

    if (auto i_result = avformat_write_header(_format_context_o.get(), nullptr); i_result < 0) {
        emit_formatted_error("Failed to write header", i_result);
        return false;
    }
    /// @todo complete here
    // if (is_eof() || _role_buffer == AudioTaskBufferType::Input) {
    //     emit_formatted_error("buffer unavailable.");
    //     return false;
    // }
    /// @todo swr
    if (auto b_planar = _codec_context_o->sample_fmt == AV_SAMPLE_FMT_S16P ||
                        _codec_context_o->sample_fmt == AV_SAMPLE_FMT_FLTP ||
                        _codec_context_o->sample_fmt == AV_SAMPLE_FMT_S32P
        ; b_planar) {
        /// @todo to process this and delete emit;
        return false;
    }

    auto FrameDeleter = [](AVFrame* f){av_frame_free(&f);};
    auto PacketDeleter = [](AVPacket* p){av_packet_free(&p);};
    std::unique_ptr<AVFrame, decltype(FrameDeleter)> frame{av_frame_alloc(), FrameDeleter};
    std::unique_ptr<AVPacket, decltype(PacketDeleter)> packet{av_packet_alloc(), PacketDeleter};

    if(!frame || !packet) {
        emit_formatted_error("Packet and Frames is null");
        return false;
    }

    int frame_size = _codec_context_o->frame_size;
    if (frame_size <= 0) frame_size = 1024;

    frame->format = _codec_context_o->sample_fmt;
    frame->ch_layout = _codec_context_o->ch_layout;
    frame->sample_rate = _codec_context_o->sample_rate;
    frame->nb_samples = frame_size;
    frame->time_base = _codec_context_o->time_base;

    av_frame_get_buffer(frame.get(), 0);

    int64_t total_bytes = 0;
    auto pcm_data = take_data();
    QByteArray pcm16;
    for (const QByteArray& chunk : pcm_data) {
        total_bytes += chunk.size();
        pcm16.append(chunk);
    }
    /// @todo complete here
    const int bytes_per_sample = 2;
    int64_t total_samples_all_channels = total_bytes / bytes_per_sample;
    int channels = _codec_context_o->ch_layout.nb_channels;
    int64_t total_samples_per_channel = total_samples_all_channels / channels;
    int64_t sample_offset = 0;


    auto index = _format_context_o->nb_streams - 1;
    while (sample_offset < total_samples_per_channel) {
        int samples_this_frame = std::min(frame_size, (int)(total_samples_per_channel - sample_offset));
        const int16_t* src = reinterpret_cast<const int16_t*>(pcm16.constData()) + sample_offset * channels;
        memcpy(frame->data[0], src, samples_this_frame * channels * sizeof(int16_t));
        if (samples_this_frame < frame_size) {
            memset(frame->data[0] + samples_this_frame * channels * sizeof(int16_t), 0,
                   (frame_size - samples_this_frame) * channels * sizeof(int16_t));
            frame->nb_samples = frame_size;
        } else {
            frame->nb_samples = frame_size;
        }
        frame->pts = sample_offset;
        sample_offset += samples_this_frame;

        int ret = avcodec_send_frame(_codec_context_o.get(), frame.get());
        if (ret < 0) { 
            /// @todo process
        }
        while (true) {
            ret = avcodec_receive_packet(_codec_context_o.get(), packet.get());
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
            if (ret < 0) { 
                /// @todo process
                break;
            }
            av_packet_rescale_ts(packet.get(), _codec_context_o->time_base, _format_context_o->streams[index]->time_base);
            packet->stream_index = index;
            av_interleaved_write_frame(_format_context_o.get(), packet.get());
            av_packet_unref(packet.get());
        }
    }

    if (auto i_result = avcodec_send_frame(_codec_context_o.get(), nullptr)
    ; i_result == 0) {
        while (true) {
            auto i_result = avcodec_receive_packet(_codec_context_o.get(), packet.get());
            if (i_result == AVERROR_EOF) break;
            if (i_result < 0) {
                /// @todo process error message
                av_packet_unref(packet.get());
                break;
            }
            av_packet_rescale_ts(packet.get(), _codec_context_o->time_base, _format_context_o->streams[index]->time_base);
            packet->stream_index = index;
            av_interleaved_write_frame(_format_context_o.get(), packet.get());
            av_packet_unref(packet.get());
        }
    }

    av_write_trailer(_format_context_o.get());

    // emit message_finished();
    return true;
}

bool FFmpegAudioTask::merge_mixing(const std::vector<TimeStampPair> &timestamp_list, const std::vector<std::string> &input_file, const QString &output_file, const FFmpegFormatConfig& config)
{
    std::cout << "merge_mixing called with output file: " << output_file.toStdString() << std::endl;
    if (mode() != AudioTaskBufferType::Output) {
        return false;
    }
    if (timestamp_list.size() != input_file.size()) {
        return false;
    }
    /// @todo check out the buffer of 'this' 
    auto pcm_data = take_data();
    QByteArray pcm16;
    for (const QByteArray& chunk : pcm_data) {
        pcm16.append(chunk);
    }
    std::cout << pcm16.size() / sizeof(int16_t) << std::endl;
    auto *p_pcmdata = reinterpret_cast<int16_t*>(pcm16.data());
    size_t total_samples = pcm16.size() / sizeof(int16_t);
    for (auto i = 0; i < timestamp_list.size(); ++i) {
        auto audiotask = FFmpegAudioTask{};
        if (auto b_result = audiotask.init(QString::fromStdString(input_file.at(i)))
        ; !b_result) {
            continue;
        }
        if(auto is_decoded = audiotask.decode(FFmpegFormatConfig{config._sample_rate, config._channel_count, config._sample_format}, false)
            ; !is_decoded) {
                audiotask.cleanup_data();
                continue;
        }
        
        auto data = audiotask.take_data();
        QByteArray cur_pcm_data;
        for (const QByteArray& chunk : data) {
            cur_pcm_data.append(chunk);
        }
        auto *p_curpcmdata = reinterpret_cast<int16_t*>(cur_pcm_data.data());
        size_t cur_samples = cur_pcm_data.size() / sizeof(int16_t);

        auto timestamp1  = timestamp_list.at(i).timestamp1;
        auto timestamp2  = timestamp_list.at(i).timestamp2;

        std::cout << timestamp1.milliseconds() << ", " << timestamp2.milliseconds() << std::endl;
        auto converted_time1 = static_cast<size_t>(timestamp1.milliseconds() * config._sample_rate / 1000 * config._channel_count);
        auto converted_time2 = static_cast<size_t>(timestamp2.milliseconds() * config._sample_rate / 1000 * config._channel_count);


        for (size_t j = converted_time1; j < converted_time2 && j < total_samples; ++j) {
            size_t src_idx = j - converted_time1;
            if (src_idx >= cur_samples) break;

            /// @brief in case overflow
            int32_t mixed = static_cast<int32_t>(p_pcmdata[j]) + static_cast<int32_t>(p_curpcmdata[src_idx]*merge_gain);
            mixed = std::clamp(mixed, 
                               static_cast<int32_t>(std::numeric_limits<int16_t>::min()), 
                               static_cast<int32_t>(std::numeric_limits<int16_t>::max()));
            p_pcmdata[j] = static_cast<int16_t>(mixed);
        }
    }
    QByteArray array_pcmdata(reinterpret_cast<const char*>(p_pcmdata), pcm16.size());
    append_data(array_pcmdata);

    auto is_encode = encode(config, output_file);
    if (!is_encode) {
        std::cout << "encode error in merge" << std::endl;
    }
    std::cout << "merge succeed" << std::endl;
    return true;
}
