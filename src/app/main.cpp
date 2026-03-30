// #include "mainwindow.h"

// #include "QtCore/qfile.h"
// #include "QtCore/qdebug.h"
// #include "QtWidgets/qapplication.h"

// int main(int argc, char *argv[])
// {
//     QApplication a(argc, argv);
//     MainWindow w;
//     QFile File_qss("../../resources/qss/stylesheet1.qss");
//     if(!File_qss.open(QFile::ReadOnly)) {
//         qDebug() << "Failed to open the stylesheet of qss.";
//         return -1;
//     }
//     QString Style_stylesheet = File_qss.readAll();
//     w.setStyleSheet(Style_stylesheet);
//     w.show();
//     return a.exec();
// }
// main.cpp
#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QFile>
#include <QGraphicsDropShadowEffect>

class CuteWindow : public QMainWindow {
public:
    CuteWindow() {
        // 加载样式
        QFile styleFile("../../resources/qss/stylesheet2.qss");
        styleFile.open(QFile::ReadOnly);
        setStyleSheet(styleFile.readAll());
        
        auto *central = new QWidget;
        auto *layout = new QVBoxLayout(central);
        layout->setSpacing(15);
        layout->setContentsMargins(30, 30, 30, 30);
        
        // 标题
        auto *title = new QLabel("💖 少女风应用");
        title->setStyleSheet("font-size: 24px; color: #ff66b3; font-weight: bold;");
        title->setAlignment(Qt::AlignCenter);
        
        // 输入框
        auto *input = new QLineEdit;
        input->setPlaceholderText("在这里输入...");
        input->setFixedHeight(40);
        
        // 按钮
        auto *btn = new QPushButton("✨ 点击我");
        btn->setFixedHeight(45);
        btn->setCursor(Qt::PointingHandCursor);
        
        // 添加阴影
        auto *shadow = new QGraphicsDropShadowEffect;
        shadow->setBlurRadius(20);
        shadow->setColor(QColor(255, 102, 179, 100));
        shadow->setOffset(0, 5);
        btn->setGraphicsEffect(shadow);
        
        layout->addWidget(title);
        layout->addWidget(input);
        layout->addWidget(btn);
        layout->addStretch();
        
        setCentralWidget(central);
        setWindowTitle("Cute App");
        resize(400, 500);
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    CuteWindow window;
    window.show();
    return app.exec();
}
