#ifndef STACKWIDGETFORDISPLAY_H
#define STACKWIDGETFORDISPLAY_H

#include <QStackedWidget>

class StackWidgetForDisplay : public QStackedWidget
{
    Q_OBJECT
public:
    StackWidgetForDisplay(QWidget *parent = nullptr);
public slots:
    void slot_ChangeCurrentPage(int currentRow);
private:
    QWidget *_page_display;
    QWidget *_page_transcription;
    QWidget *_page_translation;
    QWidget *_page_synthesis;
};

#endif // STACKWIDGETFORDISPLAY_H
