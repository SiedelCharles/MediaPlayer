#include "pagesynthesis.h"
#include "ui_pagesynthesis.h"

PageSynthesis::PageSynthesis(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageSynthesis)
{
    ui->setupUi(this);
}

PageSynthesis::~PageSynthesis()
{
    delete ui;
}
