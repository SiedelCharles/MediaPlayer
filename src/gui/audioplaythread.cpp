#include "audioplaythread.h"

#include <QDebug>
#include <QAudioFormat>

AudioPlayThread::AudioPlayThread(QObject *parent)
    : QThread(parent)
{
}

AudioPlayThread::~AudioPlayThread()
{
    Stop();
    wait();
}

void AudioPlayThread::SetFile(const QString &file_path)
{
    QMutexLocker locker(&_mutex);
    _file_path = file_path;
    _stop = false;
}

void AudioPlayThread::Stop()
{
    QMutexLocker locker(&_mutex);
    _stop = true;
    _cond.wakeAll();
}

bool AudioPlayThread::ffmpeg_init()
{
//     if (auto res = avformat_open_input(&_format_context, _file_path.toUtf8().constData(), nullptr, nullptr)
//         ; res < 0) {
//         emit SigError("Can't Open the file with error " + QString::number(res));
//         return false;
//     }
//     if (auto res = avformat_find_stream_info(_format_context, nullptr)
//         ; res < 0) {
//         emit SigError("Can't Find the message of file stream with error " + QString::number(res));
//         return false;
//     }
//     if (_stream_index = av_find_best_stream(_format_context, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0)
//         ; _stream_index < 0) {
//         emit SigError("Can't Find the file stream with error " + QString::number(_stream_index));
//         return false;
//     }
//     _time_base = _format_context->streams[_stream_index]->time_base;
//     AVCodecParameters *codec_params = _format_context->streams[_stream_index]->codecpar;
//     const AVCodec *codec = avcodec_find_decoder(codec_params->codec_id);
//     if (!codec) {
//         emit SigError("Unsupported encoding format");
//         return false;
//     }
//     _codec_context = avcodec_alloc_context3(codec);
//     if (!_codec_context) {
//         emit SigError("Failed to allocate codec context");
//         return false;
//     }
//     if (auto res = avcodec_parameters_to_context(_codec_context, codec_params)
//     ; res < 0) {
//         emit SigError("Failed to copy codec parameters with error " + QString::number(res));
//         return false;
//     }
//     if (auto res = avcodec_open2(_codec_context, codec, nullptr)
//         ; res < 0) {
//         emit SigError("Failed to open codec" + QString::number(res));
//         return false;
//     }

    // return true;
    return false;
}

void AudioPlayThread::ffmpeg_reclamation()
{
    // if (_swr_context) swr_free(&_swr_context);
    // if (_codec_context) avcodec_free_context(&_codec_context);
    // if (_format_context) avformat_close_input(&_format_context);
}

void AudioPlayThread::run()
{
    // QString file;
    // {
    //     QMutexLocker locker(&_mutex);
    //     file = _file_path;
    //     if (file.isEmpty()) {
    //         emit SigError("the Filepath is empty");
    //         return;
    //     }
    // }

    // if (!ffmpeg_init()) {
    //     ffmpeg_reclamation();
    //     return;
    // }

    // QAudioFormat format;
    // format.setSampleRate(44100);
    // format.setChannelCount(2);
    // format.setSampleFormat(QAudioFormat::Int16);

    // _audio_sink = new QAudioSink(format);
    // _audio_sink->setBufferSize(32768);

    // _audio_device = _audio_sink->start();
    // if (!_audio_device) {
    //     emit SigError("Failed to request device");
    //     ffmpeg_reclamation();
    //     delete _audio_sink;
    //     _audio_sink = nullptr;
    //     return;
    // }

    // AVChannelLayout out_ch_layout = AV_CHANNEL_LAYOUT_STEREO;
    // auto res = swr_alloc_set_opts2(&_swr_context,
    //                                &out_ch_layout,
    //                                AV_SAMPLE_FMT_S16,
    //                                44100,
    //                                &_codec_context->ch_layout,
    //                                _codec_context->sample_fmt,
    //                                _codec_context->sample_rate,
    //                                0, nullptr);
    // int ret = swr_init(_swr_context);
    // if (!_swr_context || ret < 0) {
    //     qDebug() << "swr_context is null";
    //     return;
    // }
    // if (res < 0) {
    //     emit SigError("Failed to init swr with error " + QString::number(res));
    //     ffmpeg_reclamation();
    //     _audio_sink->stop();
    //     delete _audio_sink;
    //     _audio_sink = nullptr;
    //     return;
    // }

    // AVPacket *packet = av_packet_alloc();
    // AVFrame *frame = av_frame_alloc();
    // int i = 0;
    // int sleep_index = 0;
    // while (!_stop) {
    //     ++i;
    //     int ret = av_read_frame(_format_context, packet);
    //     if (ret < 0) {
    //         if (ret == AVERROR_EOF) {
    //             break;
    //         } else {
    //             qWarning() << "av_read_frame error:" << ret;
    //             break;
    //         }
    //     }
    //     if (packet->stream_index == _stream_index) {
    //         ret = avcodec_send_packet(_codec_context, packet);
    //         if (ret < 0) {
    //             av_packet_unref(packet);
    //             continue;
    //         }
    //         while (ret >= 0 && !_stop) {
    //             ret = avcodec_receive_frame(_codec_context, frame);
    //             if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
    //                 break;
    //             } else if (ret < 0) {
    //                 break;
    //             }
    //             auto out_samples_max = frame->nb_samples;
    //             auto out_buf_size = av_samples_get_buffer_size(nullptr, 2,
    //                                                            out_samples_max,
    //                                                            AV_SAMPLE_FMT_S16, 1);
    //             if(out_buf_size < 0) {
    //                 qDebug() << "Failed to get buffer size";
    //                 av_frame_unref(frame);
    //                 break;
    //             }

    //             uint8_t *out_buffer  = (uint8_t*)av_malloc(out_buf_size);
    //             if(!out_buffer) {
    //                 qDebug() << "Failed to allocate buffer";
    //                 av_frame_unref(frame);
    //                 break;
    //             }
    //             uint8_t *output[1] = {out_buffer};
    //             int out_samples = swr_convert(_swr_context, output, frame->nb_samples,
    //                                           (const uint8_t**)frame->data, frame->nb_samples);
    //             if (out_samples < 0) {
    //                 qDebug() << "swr_convert error:" << out_samples;
    //                 av_free(out_buffer);
    //                 av_frame_unref(frame);
    //                 break;
    //             }
    //             if (frame->pts != AV_NOPTS_VALUE) {
    //                 if (_start_pts == AV_NOPTS_VALUE) {
    //                     _start_pts = frame->pts;
    //                     qDebug() << "_start_pts: " << _start_pts;
    //                     _start_time = av_gettime();
    //                 }
    //                 int64_t pts_us = av_rescale_q(frame->pts - _start_pts, _time_base, AV_TIME_BASE_Q);
    //                 int64_t elapsed_us = av_gettime() - _start_time;
    //                 int64_t sleep_us = pts_us - elapsed_us;
    //                 if (sleep_us > 0) {
    //                     ++sleep_index;
    //                     av_usleep(sleep_us);
    //                 }
    //             }
    //             if (out_samples > 0) {
    //                 int data_size = av_samples_get_buffer_size(nullptr, 2,
    //                                                            out_samples,
    //                                                            AV_SAMPLE_FMT_S16, 1);
    //                 if (data_size > 0 && _audio_device) {
    //                     _audio_device->write((const char*)output[0], data_size);
    //                 }
    //             }
    //             av_free(out_buffer);
    //             av_frame_unref(frame);
    //         }
    //     }
    //     av_packet_unref(packet);
    // }
    // qDebug() << "loop i " << i;
    // qDebug() << "sleep index " << sleep_index;
    // av_packet_free(&packet);
    // av_frame_free(&frame);
    // ffmpeg_reclamation();
    // if (_audio_sink) {
    //     _audio_sink->stop();
    //     delete _audio_sink;
    //     _audio_sink = nullptr;
    //     _audio_device = nullptr;
    // }
    // emit SigPlayFinished();
}
