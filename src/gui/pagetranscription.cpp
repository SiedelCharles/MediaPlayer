#include "pagetranscription.h"
#include "ui_pagetranscription.h"

#include "filetreeitem.h"
#include "filetreewidget.h"
#include "filetreethread.h"
#include "transcriptionthread.h"
#include "transcriptiondecodethread.h"

#include <QMenuBar>
#include <QFileDialog>

// extern "C" {
//     #include <libavutil/time.h>
//     #include <libavutil/avutil.h>
//     #include <libavcodec/avcodec.h>
//     #include <libavformat/avformat.h>
//     #include <libswresample/swresample.h>
// }

PageTranscription::PageTranscription(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageTranscription)
{
    ui->setupUi(this);
    _menu = new QMenuBar(this);

    QMenu *file_menu = _menu->addMenu("File(&F)");
    QAction *file_action = new QAction("Add File", this);
    file_action->setIcon(QIcon(":/resource/130180068_p0_master1200.jpg"));
    file_menu->addAction(file_action);
    connect(file_action, &QAction::triggered, this, &PageTranscription::slot_AddFile);

    _file_list = new FileTreeWidget(this);
    auto file_list = dynamic_cast<FileTreeWidget*>(_file_list);
    file_list->header()->hide();
    file_list->Init();
    connect(this, &PageTranscription::Sig_AddFile, file_list, &FileTreeWidget::slot_AddFile);
    _menu->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _menu->setFixedHeight(_menu->sizeHint().height());
    ui->file_layout->insertWidget(0, _menu, 0, Qt::AlignTop);
    ui->file_layout->addWidget(_file_list);

    connect(file_list, &FileTreeWidget::itemClicked, this, &PageTranscription::slot_StartThread/*dynamic_cast<AudioPlayPage*>(_audio_player), &AudioPlayPage::slot_PlayMusic*/);
    // connect(_thread_transcription, &TranscriptionThread::SigTranscriptionText
    //         , this, &PageTranscription::slot_SetText, Qt::QueuedConnection);
}

PageTranscription::~PageTranscription()
{
    delete ui;
}

void PageTranscription::slot_AddFile()
{
    QFileDialog file_dialog;
    file_dialog.setFileMode(QFileDialog::ExistingFile);
    file_dialog.setNameFilter("Audio Files (*.mp3 *.wav *.flac *.aac *.ogg)");
    file_dialog.setWindowTitle("choose file");
    file_dialog.setDirectory(QDir::currentPath());
    file_dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if(file_dialog.exec()){
        fileNames = file_dialog.selectedFiles();
    }
    if(fileNames.size()<=0) {
        return ;
    }
    QString import_path = fileNames.at(0);
    ui->output_text->setText(import_path);
    emit Sig_AddFile(import_path);
}

void PageTranscription::slot_StartThread(QTreeWidgetItem *item, int column)
{
    // qDebug() << "Start";
    // _thread_transcription = new TranscriptionThread(this);
    // _thread_transcription->SetModelPath("D:/VisualStudio_Created/VisualStudio_Resource/audio/whisper.cpp/models/ggml-small.bin");
    // _thread_decoder = new TranscriptionDecodeThread(this);
    // _thread_decoder->SetFile(item->toolTip(column));
    // connect(_thread_decoder, &TranscriptionDecodeThread::SigAudioSlices
    //         , _thread_transcription, &TranscriptionThread::AppendAudioData);
    // if(_thread_transcription) {
    //     _thread_decoder->start();
    //     _thread_transcription->start();
    // }
}

void PageTranscription::slot_SetText(const QString &text)
{
    qDebug() << "slot_SetText is executed.";
    ui->output_text->append(text);
}
