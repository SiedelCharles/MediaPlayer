/********************************************************************************
** Form generated from reading UI file 'pagedisplay.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PAGEDISPLAY_H
#define UI_PAGEDISPLAY_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PageDisplay
{
public:
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *file_layout;
    QVBoxLayout *audio_layout;

    void setupUi(QWidget *PageDisplay)
    {
        if (PageDisplay->objectName().isEmpty())
            PageDisplay->setObjectName("PageDisplay");
        PageDisplay->resize(881, 563);
        horizontalLayout = new QHBoxLayout(PageDisplay);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        file_layout = new QVBoxLayout();
        file_layout->setObjectName("file_layout");

        horizontalLayout->addLayout(file_layout);

        audio_layout = new QVBoxLayout();
        audio_layout->setObjectName("audio_layout");

        horizontalLayout->addLayout(audio_layout);

        horizontalLayout->setStretch(0, 1);
        horizontalLayout->setStretch(1, 4);

        retranslateUi(PageDisplay);

        QMetaObject::connectSlotsByName(PageDisplay);
    } // setupUi

    void retranslateUi(QWidget *PageDisplay)
    {
        PageDisplay->setWindowTitle(QCoreApplication::translate("PageDisplay", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PageDisplay: public Ui_PageDisplay {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAGEDISPLAY_H
