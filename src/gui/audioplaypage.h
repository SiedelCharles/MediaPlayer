#ifndef AUDIOPLAYPAGE_H
#define AUDIOPLAYPAGE_H
#include <QLabel>
#include <QSlider>
#include <QWidget>
#include <QThread>
#include <QPushButton>
#include <QListWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTreeWidgetItem>

class AudioPlayThread;
class AudioPlayPage : public QWidget
{
    Q_OBJECT
signals:
    void SigDetailOpenFile(QTreeWidgetItem *item, int colum);
public:
    AudioPlayPage(QWidget* parent = nullptr);
public slots :
    void slot_PlayMusic(QTreeWidgetItem *item, int column);
    void slot_ShowDetailOpenFile(QTreeWidgetItem *item, int column);
private:
    void setupUi();

    QLabel *_title_label;
    QLabel *_singer_label;

    QLabel *_picture_label;
    QPushButton *_play_btn;
    QPushButton *_pause_btn;
    QPushButton *_stop_btn;
    QPushButton *_next_btn;
    QPushButton *_prev_btn;

    QSlider *_progress_slider;

    QSlider *_volume_slider;
    QLabel *_volume_label;

    std::shared_ptr<AudioPlayThread> _play_thread = nullptr;
};

#endif // AUDIOPLAYPAGE_H
