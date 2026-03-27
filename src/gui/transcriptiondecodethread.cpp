#include "transcriptiondecodethread.h"
#include <QDebug>

TranscriptionDecodeThread::TranscriptionDecodeThread(QObject *parent)
    :AudioPlayThread(parent)
{

}

void TranscriptionDecodeThread::run()
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
    // AVChannelLayout out_ch_layout = AV_CHANNEL_LAYOUT_MONO;
    // auto res = swr_alloc_set_opts2(&_swr_context,
    //                                &out_ch_layout,
    //                                AV_SAMPLE_FMT_S16,
    //                                16000,
    //                                &_codec_context->ch_layout,
    //                                _codec_context->sample_fmt,
    //                                _codec_context->sample_rate,
    //                                0, nullptr);
    // int ret = swr_init(_swr_context);
    // if (!_swr_context || ret < 0) {
    //     qDebug() << "swr_context is null";
    //     ffmpeg_reclamation();
    //     return;
    // }
    // if (res < 0) {
    //     emit SigError("Failed to init swr with error " + QString::number(res));
    //     ffmpeg_reclamation();
    //     return ;
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
    //             auto out_buf_size = av_samples_get_buffer_size(nullptr, 1,
    //                                                            out_samples_max,
    //                                                            AV_SAMPLE_FMT_S16, 1);
    //             // qDebug() << out_buf_size;
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
    //             if (out_samples > 0) {
    //                 int data_size = av_samples_get_buffer_size(nullptr, 1, out_samples,
    //                                                            AV_SAMPLE_FMT_S16, 1);
    //                 if(data_size < 0) {
    //                     qDebug() << "data_size error";
    //                     av_free(out_buffer);
    //                     av_frame_unref(frame);
    //                     break;
    //                 }
    //                 // TODO:只考虑了小端序
    //                 QByteArray chunk(reinterpret_cast<char*>(out_buffer),
    //                                  data_size);
    //                 emit SigAudioSlices(chunk);
    //             }
    //             av_free(out_buffer);
    //             av_frame_unref(frame);
    //         }
    //     }
    //     av_packet_unref(packet);
    // }
    // av_packet_free(&packet);
    // av_frame_free(&frame);
    // ffmpeg_reclamation();
    // emit SigPlayFinished();
}


