#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

class PageTranscription;  // 前向声明，不用include
class PageTranslation;
class PageTTS;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void setupUi();
    void applyStyles();
    QWidget* createWin95Window(const QString &title, QWidget* content);

private:
    QButtonGroup    *navGroup= nullptr;
    QStackedWidget  *contentStack  = nullptr;

    // 各业务页面
    PageTranscription *_page_asr= nullptr;
    PageTranslation *_page_tr = nullptr;
    PageTTS *_page_tts = nullptr;
};