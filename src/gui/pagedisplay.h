#ifndef PAGEDISPLAY_H
#define PAGEDISPLAY_H

#include <QWidget>
#include <QMenuBar>
#include <QTreeWidgetItem>

namespace Ui {
class PageDisplay;
}

class PageDisplay : public QWidget
{
    Q_OBJECT
signals:
    void Sig_OpenFile(const QString& path);
public:
    explicit PageDisplay(QWidget *parent = nullptr);
    ~PageDisplay();
public slots:
    void slot_OpenFile(bool checked = false);
    // void slot_PlayMusic(QTreeWidgetItem *item, int column);
private:
    Ui::PageDisplay *ui;
    QMenuBar *_menu;
    QWidget *_file_list;
    QWidget *_audio_player;
};

#endif // PAGEDISPLAY_H
