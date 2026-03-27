/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actiontest1;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *layout_content;
    QHBoxLayout *layout_side;
    QHBoxLayout *layout_dialog;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1017, 684);
        actiontest1 = new QAction(MainWindow);
        actiontest1->setObjectName("actiontest1");
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        layout_content = new QHBoxLayout();
        layout_content->setObjectName("layout_content");
        layout_side = new QHBoxLayout();
        layout_side->setObjectName("layout_side");

        layout_content->addLayout(layout_side);

        layout_dialog = new QHBoxLayout();
        layout_dialog->setObjectName("layout_dialog");

        layout_content->addLayout(layout_dialog);

        layout_content->setStretch(0, 1);
        layout_content->setStretch(1, 10);

        verticalLayout->addLayout(layout_content);

        verticalLayout->setStretch(0, 10);
        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        actiontest1->setText(QCoreApplication::translate("MainWindow", "test1", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
