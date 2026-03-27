#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QPainter>
#include <QMainWindow>
#include <QStyleOption>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    virtual void paintEvent(QPaintEvent *paint_event) override;
private:
    Ui::MainWindow *ui;
    QWidget *_Widget_title;
    QWidget *_Widget_side;
    QWidget *_StackWidget_display;
private slots:
    void slot_MinimizeClose(bool checked);
    void slot_ResizeClose(bool checked);
    void slot_TitleClose(bool checked);
};
#endif // MAINWINDOW_H
