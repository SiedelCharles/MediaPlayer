#include "mainwindow.hpp"
#include "ASR_Page.hpp"   // 在cpp里include，不污染头文件
#include<QApplication>
#include "TR_page.hpp"
#include "TTS_page.hpp"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUi();
    applyStyles();
    setWindowTitle("Windose");
    resize(1000, 700);
}

void MainWindow::setupUi() {
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setObjectName("desktopBackground");
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    // --- 左侧侧边栏 ---
    QWidget *sidebar = new QWidget();
    sidebar->setFixedWidth(180);
    QVBoxLayout *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(0, 0, 0, 0);
    sideLayout->setSpacing(15);

    navGroup = new QButtonGroup(this);
    QStringList menuItems = {"音频播放", "语音识别", "文本翻译", "语音合成"};

    for (int i = 0; i < menuItems.size(); ++i) {
        QPushButton *btn = new QPushButton("■ " + menuItems[i]);
        btn->setCheckable(true);
        btn->setObjectName("desktopIconBtn");
        navGroup->addButton(btn, i);
        sideLayout->addWidget(btn);
    }
    navGroup->button(0)->setChecked(true);
    sideLayout->addStretch();

    // --- 右侧内容区 ---
    contentStack = new QStackedWidget();
    contentStack->setObjectName("contentArea");

    // page0:音频播放（暂时占位）
    QWidget *page0 = new QWidget();
    page0->setObjectName("page0");
    contentStack->addWidget(page0);

    // page1: 语音识别 - PageTranscription
    _page_asr = new PageTranscription(this);
    contentStack->addWidget(_page_asr);

    // page2: 文本翻译（暂时占位）
    _page_tr = new PageTranslation(this);
    contentStack->addWidget(_page_tr);

    // page3: 语音合成（暂时占位）
    class PageTTS *page_tts = new PageTTS(this);
    contentStack->addWidget(page_tts);

    //默认显示第0页
    contentStack->setCurrentIndex(0);

    QWidget *windoseApp = createWin95Window("应用管理器", contentStack);

    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(windoseApp, 1);

    connect(navGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            contentStack, &QStackedWidget::setCurrentIndex);
}

QWidget* MainWindow::createWin95Window(const QString &title, QWidget* content) {
    QWidget *win = new QWidget();
    win->setObjectName("nsoWindow");

    QVBoxLayout *layout = new QVBoxLayout(win);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(0);

    QWidget *titleBar = new QWidget();
    titleBar->setObjectName("nsoTitleBar");
    titleBar->setFixedHeight(26);
    QHBoxLayout *tLayout = new QHBoxLayout(titleBar);
    tLayout->setContentsMargins(8, 0, 4, 0);
    tLayout->setSpacing(2);

    QLabel *tLabel = new QLabel("■ " + title);
    tLabel->setObjectName("nsoTitleLabel");

    QPushButton *minBtn   = new QPushButton("一");
    QPushButton *maxBtn   = new QPushButton("□");
    QPushButton *closeBtn = new QPushButton("×");
    minBtn->setObjectName("nsoCtrlBtn");
    maxBtn->setObjectName("nsoCtrlBtn");
    closeBtn->setObjectName("nsoCtrlBtn");

    tLayout->addWidget(tLabel);
    tLayout->addStretch();
    tLayout->addWidget(minBtn);
    tLayout->addWidget(maxBtn);
    tLayout->addWidget(closeBtn);

    layout->addWidget(titleBar);
    layout->addWidget(content, 1);

    return win;
}

void MainWindow::applyStyles() {
    QString style = R"(
        #desktopBackground {
            background-color: #d8f5f5;
        }
        #desktopIconBtn {
            background-color: rgba(255, 255, 255, 0.6);
            border: 2px solid #a28ccb;
            color: #a28ccb;
            font-family: 'Microsoft YaHei';
            font-weight: bold;
            font-size: 14px;
            padding: 12px;
            text-align: left;
            border-radius: 4px;
        }
        #desktopIconBtn:hover {
            background-color: #fce4ec;
            color: #ff80ab;
            border-color: #ff80ab;
        }
        #desktopIconBtn:checked {
            background-color: #a28ccb;
            color: white;
            border: 2px solid #5ce1e6;
            font-weight: 900;
        }
        #nsoWindow {
            background-color: white;
            border: 3px solid #5ce1e6;
        }
        #nsoTitleBar {
            background-color: #a28ccb;
            border-bottom: 2px solid #5ce1e6;
        }
        #nsoTitleLabel {
            color: white;
            font-weight: bold;
            font-size: 13px;
        }
        #nsoCtrlBtn {
            background-color: transparent;
            color: white;
            border: 1px solid transparent;
            font-size: 16px;
            font-weight: bold;
            width: 24px;
            height: 20px;
        }
        #nsoCtrlBtn:hover {
            background-color: rgba(255, 255, 255, 0.3);
            border: 1px solid white;
        }
        #contentArea {
            background-color: #fff0f5;
            border: none;
        }
    )";
    this->setStyleSheet(style);
}
