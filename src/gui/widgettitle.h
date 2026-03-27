#ifndef WIDGETTITLE_H
#define WIDGETTITLE_H

#include <QWidget>
#include <QMouseEvent>
#include <QPushButton>
class MainWndow;

class WidgetTitle : public QWidget
{
    Q_OBJECT
signals:
    void signal_slot_TitleMinimizeClicked(bool checked);
    void signal_slot_TitleResizeClicked(bool checked);
    void signal_slot_TitleCloseClicked(bool checked);
public:
    explicit WidgetTitle(QWidget *parent = nullptr);
protected:
    // void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
private:
    QPushButton *_Btn_minimize;
    QPushButton *_Btn_resize;
    QPushButton *_Btn_close;
    bool _clicked;
    QPoint _offset;
private slots:
    void slot_slot_TitleMinimizeClicked(bool checked);
    void slot_slot_TitleResizelicked(bool checked);
    void slot_slot_TitleCloseClicked(bool checked);
};

#endif // WIDGETTITLE_H
