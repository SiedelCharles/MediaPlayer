#include "audioplaypage.h"
#include <QDir>
#include <QLabel>
#include <QWidget>
#include <QSlider>
#include <QPushButton>
#include <QListWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "audioplaythread.h"
// extern "C" {
// #include <libavcodec/avcodec.h>
// }

AudioPlayPage::AudioPlayPage(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    connect(this, &AudioPlayPage::SigDetailOpenFile, this, &AudioPlayPage::slot_ShowDetailOpenFile);
}

void AudioPlayPage::slot_PlayMusic(QTreeWidgetItem *item, int column)
{
    if (!item) return;
    QString file_path = item->data(0, Qt::ToolTipRole).toString();
    if (file_path.isEmpty()) {
        return;
    }
    if(_play_thread && !_play_thread->IsStop()) {
        _play_thread->Stop();
    }
    emit SigDetailOpenFile(item, column);
    _play_thread = std::make_shared<AudioPlayThread>();
    _play_thread->SetFile(file_path);
    _play_thread->start();
}

void AudioPlayPage::slot_ShowDetailOpenFile(QTreeWidgetItem *item, int column)
{
    // if (!item) return;
    // QString file_path = item->data(0, Qt::ToolTipRole).toString();
    // if (file_path.isEmpty()) {
    //     return;
    // }
    // QDir dir(file_path);
    // auto file_name = dir.dirName();
    // _title_label->setText(file_name);
}

void AudioPlayPage::setupUi()
{
    QVBoxLayout *main_layout = new QVBoxLayout(this);

    QHBoxLayout *info_layout = new QHBoxLayout();
    _title_label = new QLabel(tr("Title"), this);
    _singer_label = new QLabel(tr("Singer"), this);
    _title_label->setFixedHeight(40);
    _singer_label->setFixedHeight(40);
    info_layout->addWidget(_title_label);
    info_layout->addWidget(_singer_label);
    info_layout->addStretch();
    main_layout->addLayout(info_layout);

    _picture_label = new QLabel(this);
    _picture_label->setText("test");
    _picture_label->setStyleSheet("background-color:#CCCCFF");
    // auto pximap = QPixmap(":/resource/processed1.jpg");
    // _picture_label->setPixmap(pximap);
    // _picture_label->setScaledContents(true);
    main_layout->addWidget(_picture_label);

    QHBoxLayout *control_layout = new QHBoxLayout();

    _play_btn  = new QPushButton(tr("上一曲"), this);
    _pause_btn = new QPushButton(tr("播放"), this);
    _stop_btn  = new QPushButton(tr("暂停"), this);
    _next_btn  = new QPushButton(tr("停止"), this);
    _prev_btn  = new QPushButton(tr("下一曲"), this);

    control_layout->addWidget(_play_btn);
    control_layout->addWidget(_pause_btn);
    control_layout->addWidget(_stop_btn);
    control_layout->addWidget(_next_btn);
    control_layout->addWidget(_prev_btn);
    control_layout->addStretch();

    _progress_slider = new QSlider(Qt::Horizontal, this);
    _progress_slider->setRange(0, 100);
    _progress_slider->setValue(0);

    QHBoxLayout *volume_layout = new QHBoxLayout();
    _volume_label = new QLabel(tr("音量"), this);
    _volume_slider = new QSlider(Qt::Horizontal, this);
    _volume_slider->setRange(0, 100);
    _volume_slider->setValue(50);
    volume_layout->addWidget(_volume_label);
    volume_layout->addWidget(_volume_slider);

    QVBoxLayout *bottom_layout = new QVBoxLayout();
    bottom_layout->addWidget(_progress_slider);
    bottom_layout->addLayout(control_layout);
    bottom_layout->addLayout(volume_layout);

    main_layout->addLayout(bottom_layout);

    setLayout(main_layout);
}

