/********************************************************************************
** Form generated from reading UI file 'pagetranslation.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PAGETRANSLATION_H
#define UI_PAGETRANSLATION_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PageTranslation
{
public:

    void setupUi(QWidget *PageTranslation)
    {
        if (PageTranslation->objectName().isEmpty())
            PageTranslation->setObjectName("PageTranslation");
        PageTranslation->resize(400, 300);

        retranslateUi(PageTranslation);

        QMetaObject::connectSlotsByName(PageTranslation);
    } // setupUi

    void retranslateUi(QWidget *PageTranslation)
    {
        PageTranslation->setWindowTitle(QCoreApplication::translate("PageTranslation", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PageTranslation: public Ui_PageTranslation {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAGETRANSLATION_H
