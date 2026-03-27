#ifndef WIDGETSIDE_H
#define WIDGETSIDE_H

#include <QWidget>
#include <QListWidget>
#include <QListWidgetItem>

class WidgetSide : public QListWidget
{
public:
    WidgetSide(QWidget *parent = nullptr);
private:
    QListWidgetItem *_item_display;
    QListWidgetItem *_item_transcription;
    QListWidgetItem *_item_translation;
    QListWidgetItem *_item_synthesis;
};

#endif // WIDGETSIDE_H
