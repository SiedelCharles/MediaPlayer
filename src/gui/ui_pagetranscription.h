/********************************************************************************
** Form generated from reading UI file 'pagetranscription.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PAGETRANSCRIPTION_H
#define UI_PAGETRANSCRIPTION_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PageTranscription
{
public:
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *file_layout;
    QVBoxLayout *text_layout;
    QTextBrowser *output_text;

    void setupUi(QWidget *PageTranscription)
    {
        if (PageTranscription->objectName().isEmpty())
            PageTranscription->setObjectName("PageTranscription");
        PageTranscription->resize(969, 565);
        horizontalLayout = new QHBoxLayout(PageTranscription);
        horizontalLayout->setObjectName("horizontalLayout");
        file_layout = new QVBoxLayout();
        file_layout->setObjectName("file_layout");

        horizontalLayout->addLayout(file_layout);

        text_layout = new QVBoxLayout();
        text_layout->setObjectName("text_layout");
        output_text = new QTextBrowser(PageTranscription);
        output_text->setObjectName("output_text");

        text_layout->addWidget(output_text);


        horizontalLayout->addLayout(text_layout);

        horizontalLayout->setStretch(0, 1);
        horizontalLayout->setStretch(1, 4);

        retranslateUi(PageTranscription);

        QMetaObject::connectSlotsByName(PageTranscription);
    } // setupUi

    void retranslateUi(QWidget *PageTranscription)
    {
        PageTranscription->setWindowTitle(QCoreApplication::translate("PageTranscription", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PageTranscription: public Ui_PageTranscription {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAGETRANSCRIPTION_H
