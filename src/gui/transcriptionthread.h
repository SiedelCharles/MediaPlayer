#ifndef TRANSCRIPTIONTHREAD_H
#define TRANSCRIPTIONTHREAD_H

#include <QMutex>
#include <QThread>
#include <QWaitCondition>

// extern "C" {
//     #include "whisper.h"
// }

class TranscriptionThread : public QThread
{
    Q_OBJECT
signals:
    void SigTranscriptionText(const QString& text);
public:
    explicit TranscriptionThread(QObject *parent = nullptr);
    ~TranscriptionThread();
    void Stop();

    void SetModelPath(const QString& path);
    // void SetFilePath(const QString& path);
    void SetInterval(const int intervals);
    void SetAudioParams(int sample_rate, int channel_count, int sample_size);
    void AppendAudioData(const QByteArray &data);
    QVector<float> ConvertToFloat32(const QByteArray &pcm_data);
protected:
    virtual void run() override;
private:
    bool _stop = false;
    QWaitCondition _cond;
    QMutex _mutex;
    // QString _file_path;
    QString _model_path;
    // struct whisper_context *_whisper_context = nullptr;

    int _sample_rate = 16000;
    int _channel_count = 1;
    int _sample_size = 16;

    int _intervals = 1;
    QList<QByteArray> _audio_queue;
};



#endif // TRANSCRIPTIONTHREAD_H
