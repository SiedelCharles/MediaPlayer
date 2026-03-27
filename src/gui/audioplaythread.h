#ifndef AUDIOPLAYTHREAD_H
#define AUDIOPLAYTHREAD_H

#include <QMutex>
#include <QThread>
#include <QByteArray>
#include <QAudioSink>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QWaitCondition>

// extern "C" {
//     #include <libavutil/time.h>
//     #include <libavutil/avutil.h>
//     #include <libavcodec/avcodec.h>
//     #include <libavformat/avformat.h>
//     #include <libswresample/swresample.h>
// }

class AudioPlayThread : public QThread
{
    Q_OBJECT
public:
    explicit AudioPlayThread(QObject *parent = nullptr);
    ~AudioPlayThread();

    void SetFile(const QString &file_path);
    void Stop();
    bool IsStop();
signals:
    void SigPlayFinished();
    void SigError(const QString &msg);
protected:
    virtual void run() override;
// private:
    QString _file_path;
    QMutex _mutex;
    QWaitCondition _cond;
    bool _stop = false;

    // SwrContext *_swr_context = nullptr;
    // AVCodecContext *_codec_context = nullptr;
    // AVFormatContext *_format_context = nullptr;
    // int _stream_index = -1;

    // int64_t _start_pts = AV_NOPTS_VALUE;
    // int64_t _start_time = 0;
    // AVRational _time_base;
    bool ffmpeg_init();
    void ffmpeg_reclamation();
private:
    QIODevice *_audio_device = nullptr;
    QAudioSink *_audio_sink = nullptr;
};

inline bool AudioPlayThread::IsStop() {
    return _stop;
}

#endif // AUDIOPLAYTHREAD_H
