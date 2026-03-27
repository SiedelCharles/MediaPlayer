#include "widgettitle.h"

#include <Qpainter>
#include <QHBoxLayout>
WidgetTitle::WidgetTitle(QWidget *parent)
    : QWidget{parent}, _clicked(false)
{
    setAttribute(Qt::WA_StyledBackground, true);

    _Btn_minimize = new QPushButton("-", this);
    _Btn_resize = new QPushButton("□", this);
    _Btn_close = new QPushButton("×", this);

    _Btn_minimize->setFixedSize(40, 30);
    _Btn_resize->setFixedSize(40, 30);
    _Btn_close->setFixedSize(40, 30);

    connect(_Btn_minimize, &QPushButton::clicked, this, &WidgetTitle::slot_slot_TitleMinimizeClicked);
    connect(_Btn_resize, &QPushButton::clicked, this, &WidgetTitle::slot_slot_TitleResizelicked);
    connect(_Btn_close, &QPushButton::clicked, this, &WidgetTitle::slot_slot_TitleCloseClicked);

    QHBoxLayout *layout1 = new QHBoxLayout(this);
    layout1->addStretch();
    layout1->addWidget(_Btn_minimize);
    layout1->addWidget(_Btn_resize);
    layout1->addWidget(_Btn_close);

    setLayout(layout1);
}

void WidgetTitle::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton) {
        _clicked = true;
        _offset = event->globalPos() - parentWidget()->pos();
    }
}

void WidgetTitle::mouseReleaseEvent(QMouseEvent *event)
{
    _clicked = false;
}

void WidgetTitle::mouseMoveEvent(QMouseEvent *event)
{
    if((event->buttons() & Qt::LeftButton) && _clicked) {
        this->parentWidget()->move(event->globalPos() - _offset);
    }
}

void WidgetTitle::slot_slot_TitleMinimizeClicked(bool checked)
{
    emit signal_slot_TitleMinimizeClicked(checked);
}

void WidgetTitle::slot_slot_TitleResizelicked(bool checked)
{
    emit signal_slot_TitleResizeClicked(checked);
}

void WidgetTitle::slot_slot_TitleCloseClicked(bool checked)
{
    emit signal_slot_TitleCloseClicked(checked);
}

