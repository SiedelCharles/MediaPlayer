#ifndef TRANCSRIPTIONDECODETHREAD_H
#define TRANCSRIPTIONDECODETHREAD_H
#include "audioplaythread.h"

class TranscriptionDecodeThread : public AudioPlayThread
{
    Q_OBJECT
signals:
    void SigAudioSlices(const QByteArray& data);
public:
    TranscriptionDecodeThread(QObject *parent = nullptr);
protected:
    virtual void run() override;
};

#endif // TRANCSRIPTIONDECODETHREAD_H
