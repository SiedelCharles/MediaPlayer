#ifndef PAGETRANSLATION_H
#define PAGETRANSLATION_H

#include <QWidget>

namespace Ui {
class PageTranslation;
}

class PageTranslation : public QWidget
{
    Q_OBJECT

public:
    explicit PageTranslation(QWidget *parent = nullptr);
    ~PageTranslation();

private:
    Ui::PageTranslation *ui;
};

#endif // PAGETRANSLATION_H
