#include "transcriptionthread.h"
#include <qdebug.h>

TranscriptionThread::TranscriptionThread(QObject *parent)
    : QThread{parent}
{}

TranscriptionThread::~TranscriptionThread()
{
    // Stop();
    // wait();
    // if(_whisper_context) {
    //     whisper_free(_whisper_context);
    //     _whisper_context = nullptr;
    // }
}

void TranscriptionThread::Stop()
{
    QMutexLocker locker(&_mutex);
    _stop = true;
}

void TranscriptionThread::SetModelPath(const QString& path)
{
    _model_path = path;
}

// void TranscriptionThread::SetFilePath(const QString& path)
// {
//     _file_path = path;
// }

void TranscriptionThread::SetInterval(const int intervals)
{
    _intervals = intervals;
}

void TranscriptionThread::SetAudioParams(int sample_rate, int channel_count, int sample_size)
{
    _sample_rate = sample_rate;
    _channel_count = channel_count;
    _sample_size = sample_size;
}

void TranscriptionThread::AppendAudioData(const QByteArray &data)
{
    QMutexLocker locker(&_mutex);
    _audio_queue.append(data);
    _cond.wakeOne();
}

QVector<float> TranscriptionThread::ConvertToFloat32(const QByteArray &pcm_data)
{
    QVector<float> pcm_f32;
    pcm_f32.reserve(pcm_data.size() >> 1);

    const int16_t *pcm_16 = reinterpret_cast<const int16_t*>(pcm_data.constData());
    int sample_count = pcm_data.size() / sizeof(int16_t);

    for (int i = 0; i < sample_count; ++i) {
        pcm_f32.push_back(static_cast<float>(pcm_16[i]) / 32768.0f);
    }
    return pcm_f32;
}

void TranscriptionThread::run()
{
    // if (_model_path.isEmpty()) {
    //     // emit error signal;
    //     return;
    // }

    // QByteArray path_bytes = _model_path.toUtf8();
    // _whisper_context = whisper_init_from_file(path_bytes.constData());
    // if (!_whisper_context) {
    //     // emit error signal;
    //     return;
    // }

    // whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    // params.print_realtime   = false;
    // params.print_progress   = false;
    // params.print_timestamps = true;
    // params.print_special    = false;
    // params.translate        = false;
    // params.language         = "ja";
    // params.n_threads        = 4;
    // params.offset_ms        = 0;
    // params.duration_ms      = 0;

    // int bytes_per_second = _sample_rate * (_sample_size / 8) * _channel_count;
    // QByteArray accumulated_audio;
    // while (!_stop) {
    //     {
    //         QMutexLocker locker(&_mutex);
    //         while (_audio_queue.isEmpty() && !_stop) {
    //             _cond.wait(&_mutex);
    //         }
    //         if (_stop && _audio_queue.isEmpty()) {
    //             break;
    //         }
    //         for (const QByteArray &data : qAsConst(_audio_queue)) {
    //             accumulated_audio.append(data);
    //         }
    //         _audio_queue.clear();
    //     }

    //     if (accumulated_audio.size() < _intervals * bytes_per_second) {
    //         continue;
    //     }
    //     qDebug() << accumulated_audio.size();
    //     QVector<float> pcm_f32 = ConvertToFloat32(accumulated_audio);
    //     accumulated_audio.clear();

    //     int ret = whisper_full(_whisper_context, params, pcm_f32.data(), pcm_f32.size());
    //     if (ret != 0) {
    //         // emit error signal;
    //         continue;
    //     }

    //     int n_segments = whisper_full_n_segments(_whisper_context);
    //     QString result_text;
    //     for (int i = 0; i < n_segments; ++i) {
    //         const char *text = whisper_full_get_segment_text(_whisper_context, i);
    //         int64_t t0 = whisper_full_get_segment_t0(_whisper_context, i);
    //         int64_t t1 = whisper_full_get_segment_t1(_whisper_context, i);
    //         QString timestamp = QString("[%1.%2 --> %3.%4]")
    //                                 .arg(t0/1000).arg(t0%1000, 3, 10, QChar('0'))
    //                                 .arg(t1/1000).arg(t1%1000, 3, 10, QChar('0'));
    //         result_text += timestamp + "\n" + QString::fromUtf8(text);
    //     }

    //     if (!result_text.isEmpty()) {
    //         qDebug() << result_text;
    //         emit SigTranscriptionText(result_text);
    //         qDebug() << "signal is called.";
    //     }
    // }

    // if (!accumulated_audio.isEmpty()) {
    //     QVector<float> pcm_f32 = ConvertToFloat32(accumulated_audio);
    //     int ret = whisper_full(_whisper_context, params, pcm_f32.data(), pcm_f32.size());
    //     if (ret == 0) {
    //         int n_segments = whisper_full_n_segments(_whisper_context);
    //         QString final_text;
    //         for (int i = 0; i < n_segments; ++i) {
    //             final_text += QString::fromUtf8(whisper_full_get_segment_text(_whisper_context, i));
    //         }
    //         if (!final_text.isEmpty()) {
    //             qDebug() << final_text;
    //             emit SigTranscriptionText(final_text);
    //         }
    //     }
    // }
    // whisper_free(_whisper_context);
    // _whisper_context = nullptr;
}

