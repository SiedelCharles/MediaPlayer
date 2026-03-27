#ifndef PAGESYNTHESIS_H
#define PAGESYNTHESIS_H

#include <QWidget>

namespace Ui {
class PageSynthesis;
}

class PageSynthesis : public QWidget
{
    Q_OBJECT

public:
    explicit PageSynthesis(QWidget *parent = nullptr);
    ~PageSynthesis();

private:
    Ui::PageSynthesis *ui;
};

#endif // PAGESYNTHESIS_H
