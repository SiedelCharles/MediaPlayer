#include "pagetranslation.h"
#include "ui_pagetranslation.h"

PageTranslation::PageTranslation(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageTranslation)
{
    ui->setupUi(this);
}

PageTranslation::~PageTranslation()
{
    delete ui;
}
