#include "mainwindow.h"

#include <QFile>
#include <QDebug>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QFile File_qss(":/qss/stylesheet1.qss");
    if(!File_qss.open(QFile::ReadOnly)) {
        qDebug() << "Failed to open the stylesheet of qss.";
        return -1;
    }
    QString Style_stylesheet = File_qss.readAll();
    w.setStyleSheet(Style_stylesheet);
    w.show();
    return a.exec();
}
