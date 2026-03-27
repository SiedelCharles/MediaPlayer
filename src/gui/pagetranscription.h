#ifndef PAGETRANSCRIPTION_H
#define PAGETRANSCRIPTION_H

#include <QWidget>
#include <QMenuBar>
#include <QTreeWidgetItem>

#include "transcriptionthread.h"
#include "transcriptiondecodethread.h"

namespace Ui {
class PageTranscription;
}

class PageTranscription : public QWidget
{
    Q_OBJECT
signals:
    void Sig_AddFile(const QString path);
public:
    explicit PageTranscription(QWidget *parent = nullptr);
    ~PageTranscription();
public slots:
    void slot_AddFile();
    void slot_StartThread(QTreeWidgetItem *item, int column);
    // void slot_TerminateThread();
    void slot_SetText(const QString& text);
private:
    Ui::PageTranscription *ui;
    QMenuBar *_menu;
    QWidget *_file_list;
    // TranscriptionThread *_thread_transcription = nullptr;
    // TranscriptionDecodeThread *_thread_decoder = nullptr;
};

#endif // PAGETRANSCRIPTION_H
