/********************************************************************************
** Form generated from reading UI file 'pagesynthesis.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PAGESYNTHESIS_H
#define UI_PAGESYNTHESIS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PageSynthesis
{
public:

    void setupUi(QWidget *PageSynthesis)
    {
        if (PageSynthesis->objectName().isEmpty())
            PageSynthesis->setObjectName("PageSynthesis");
        PageSynthesis->resize(400, 300);

        retranslateUi(PageSynthesis);

        QMetaObject::connectSlotsByName(PageSynthesis);
    } // setupUi

    void retranslateUi(QWidget *PageSynthesis)
    {
        PageSynthesis->setWindowTitle(QCoreApplication::translate("PageSynthesis", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PageSynthesis: public Ui_PageSynthesis {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAGESYNTHESIS_H
