#pragma push_macro("slots")
#undef slots
#include "TR/LLM/translate_script.h"
#pragma pop_macro("slots")
#include "TR_page.hpp"
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>

static py::scoped_interpreter* g_guard = nullptr;

// static void ensurePythonInit() {
//     // 必须在主线程调用
//     Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());
//     if (!g_guard) {
//         Py_SetPythonHome(
//             L"D:/VisualStudio_Created/VisualStudio_Resource/"
//             L"miniconda/envs/audio"
//         );
//         g_guard = new py::scoped_interpreter();
        
//         //  初始化后立即释放 GIL，允许子线程通过 gil_scoped_acquire 获取
//         PyEval_SaveThread();
//     }
// }

TranslateWorker::TranslateWorker(const QString &text,
                                 const QString &srcLang,
                                 const QString &dstLang,
                                 const QString &prompt,
                                 QObject *parent)
    : QObject(parent), m_text(text),
      m_srcLang(srcLang), m_dstLang(dstLang),  m_prompt(prompt) {}

void TranslateWorker::doTranslate() {
    try {
        QString result;
            {
            // ✅ 显式获取 GIL
            py::gil_scoped_acquire gil;

            py::module sys = py::module::import("sys");
            std::string scriptDir =
                "D:/VisualStudio_Created/VisualStudio_Project/"
                "Projects/MediaPlayer/src/TR/LLM";
            sys.attr("path").attr("append")(scriptDir);

            py::module_ s_script = py::module::import("translate_script");

            std::string inputText = m_text.toStdString();
            std::string srcLang   = m_srcLang.toStdString();
            std::string dstLang   = m_dstLang.toStdString();
            std::string prompt    = m_prompt.toStdString();

            py::object pyResult = s_script.attr("translate_llm_deepseek")(
                inputText, srcLang, dstLang, prompt
            );
            result = QString::fromStdString(
                pyResult.cast<std::string>()
            );
        }
        // ✅ 在释放 GIL 前，确保所有 py:: 对象已完成使用
        emit finished(result);

    } catch (const py::error_already_set &e) {
        emit errorOccurred(
            QString("Python 错误：%1").arg(e.what())
        );
    } catch (const std::exception &e) {
        emit errorOccurred(
            QString("异常：%1").arg(e.what())
        );
    }
}


PageTranslation::PageTranslation(QWidget *parent) : QWidget(parent) {
    // ensurePythonInit();
    setupUi();
    applyStyles();
}

void PageTranslation::setupUi() {
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(12);

    // ── 顶部标题栏 ──────────────────────────────
    QLabel *titleLabel = new QLabel("✦ 文本翻译");
    titleLabel->setObjectName("transTitleLabel");
    root->addWidget(titleLabel);

    // ── 语言选择行 ──────────────────────────────
    QHBoxLayout *langRow = new QHBoxLayout();
    langRow->setSpacing(10);

    QStringList languages;
    languages << "中文（简体）" << "中文（繁体）" << "英语" << "日语"
            << "韩语" << "法语" << "德语" << "西班牙语" << "俄语" << "阿拉伯语";

    // 源语言
    QLabel *srcLabel = new QLabel("源语言：");
    srcLabel->setObjectName("transLangLabel");
    srcLangBox = new QComboBox();
    srcLangBox->setObjectName("transComboBox");
    srcLangBox->addItems(languages);
    srcLangBox->setCurrentIndex(3);   // 默认：日语

    // 互换按钮
    swapBtn = new QPushButton("⇌");
    swapBtn->setObjectName("transSwapBtn");
    swapBtn->setToolTip("互换语言");
    swapBtn->setFixedSize(36, 36);

    // 目标语言
    QLabel *dstLabel = new QLabel("目标语言：");
    dstLabel->setObjectName("transLangLabel");
    dstLangBox = new QComboBox();
    dstLangBox->setObjectName("transComboBox");
    dstLangBox->addItems(languages);
    dstLangBox->setCurrentIndex(0);   // 默认：中文

    langRow->addWidget(srcLabel);
    langRow->addWidget(srcLangBox, 2);
    langRow->addWidget(swapBtn);
    langRow->addWidget(dstLabel);
    langRow->addWidget(dstLangBox, 2);
    langRow->addStretch();
    root->addLayout(langRow);

    // ── Prompt 设置区 ──────────────────────────────
    QWidget *promptPanel = new QWidget();
    promptPanel->setObjectName("transPromptPanel");
    QVBoxLayout *promptLayout = new QVBoxLayout(promptPanel);
    promptLayout->setContentsMargins(0, 0, 0, 0);
    promptLayout->setSpacing(4);

    QHBoxLayout *promptHeader = new QHBoxLayout();

    QLabel *promptLabel = new QLabel("✦ 上下文提示（Prompt）");
    promptLabel->setObjectName("transPromptLabel");

    // 折叠/展开按钮
    QPushButton *promptToggle = new QPushButton("▾ 展开");
    promptToggle->setObjectName("transPromptToggle");
    promptToggle->setFixedSize(70, 24);
    promptToggle->setCheckable(true);

    promptHeader->addWidget(promptLabel);
    promptHeader->addStretch();
    promptHeader->addWidget(promptToggle);

    promptEdit = new QTextEdit();
    promptEdit->setObjectName("transPromptEdit");
    promptEdit->setPlaceholderText(
        "（可选）描述文本背景，帮助 AI 更准确翻译，例如：这段文本与奇幻小说有关…");
    promptEdit->setAcceptRichText(false);
    promptEdit->setFixedHeight(72);
    promptEdit->setPlainText(
        "这段文本可能与触手，魔法少女(cure pink)，苗床，产卵有关");  // 默认值
    promptEdit->hide();   // 默认折叠

    promptLayout->addLayout(promptHeader);
    promptLayout->addWidget(promptEdit);

    root->addWidget(promptPanel);

    // 折叠/展开逻辑
    connect(promptToggle, &QPushButton::toggled,
            this, [this, promptToggle](bool checked) {
                promptEdit->setVisible(checked);
                promptToggle->setText(checked ? "▴ 收起" : "▾ 展开");
            });


    // ── 文本区（左右分栏）────────────────────────
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->setObjectName("transSplitter");
    splitter->setHandleWidth(6);

    // 左：输入区
    QWidget *inputPanel = new QWidget();
    inputPanel->setObjectName("transPanel");
    QVBoxLayout *inLayout = new QVBoxLayout(inputPanel);
    inLayout->setContentsMargins(0, 0, 0, 0);
    inLayout->setSpacing(6);

    QLabel *inputHeader = new QLabel("▸ 原文");
    inputHeader->setObjectName("transPanelHeader");
    inputEdit = new QTextEdit();
    inputEdit->setObjectName("transInputEdit");
    inputEdit->setPlaceholderText("请在此输入要翻译的文本…");
    inputEdit->setAcceptRichText(false);

    charCountLabel = new QLabel("0 / 5000 字符");
    charCountLabel->setObjectName("transCharCount");
    charCountLabel->setAlignment(Qt::AlignRight);

    inLayout->addWidget(inputHeader);
    inLayout->addWidget(inputEdit, 1);
    inLayout->addWidget(charCountLabel);

    // 右：输出区
    QWidget *outputPanel = new QWidget();
    outputPanel->setObjectName("transPanel");
    QVBoxLayout *outLayout = new QVBoxLayout(outputPanel);
    outLayout->setContentsMargins(0, 0, 0, 0);
    outLayout->setSpacing(6);

    QLabel *outputHeader = new QLabel("▸ 译文");
    outputHeader->setObjectName("transPanelHeader");
    outputEdit = new QTextEdit();
    outputEdit->setObjectName("transOutputEdit");
    outputEdit->setPlaceholderText("翻译结果将显示在这里…");
    outputEdit->setReadOnly(true);

    copyBtn = new QPushButton("⧉ 复制译文");
    copyBtn->setObjectName("transCopyBtn");
    copyBtn->setFixedHeight(30);

    outLayout->addWidget(outputHeader);
    outLayout->addWidget(outputEdit, 1);
    outLayout->addWidget(copyBtn);

    splitter->addWidget(inputPanel);
    splitter->addWidget(outputPanel);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);
    root->addWidget(splitter, 1);

    // ── 底部操作行 ──────────────────────────────
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->setSpacing(10);

    statusLabel = new QLabel("就绪");
    statusLabel->setObjectName("transStatusLabel");

    clearBtn = new QPushButton("✕ 清空");
    clearBtn->setObjectName("transClearBtn");
    clearBtn->setFixedSize(90, 36);

    translateBtn = new QPushButton("▶ 开始翻译");
    translateBtn->setObjectName("transStartBtn");
    translateBtn->setFixedSize(120, 36);

    btnRow->addWidget(statusLabel);
    btnRow->addStretch();
    btnRow->addWidget(clearBtn);
    btnRow->addWidget(translateBtn);
    root->addLayout(btnRow);

    // ── 信号连接 ────────────────────────────────
    connect(translateBtn, &QPushButton::clicked,
            this, &PageTranslation::onTranslateClicked);
    connect(swapBtn, &QPushButton::clicked,
            this, &PageTranslation::onSwapLanguages);
    connect(clearBtn, &QPushButton::clicked,
            this, &PageTranslation::onClearClicked);
    connect(copyBtn, &QPushButton::clicked,
            this, &PageTranslation::onCopyResult);
    connect(inputEdit, &QTextEdit::textChanged,
            this, &PageTranslation::updateCharCount);
}

// ── Slots ────────────────────────────────────────

void PageTranslation::onTranslateClicked() {
    QString text = inputEdit->toPlainText().trimmed();
    if (text.isEmpty()) {
        statusLabel->setText("⚠ 请先输入文本");
        return;
    }
    if (text.length() > 5000) {
        statusLabel->setText("⚠ 文本超过 5000 字符限制");
        return;
    }

    // 禁用按钮，显示状态
    translateBtn->setEnabled(false);
    clearBtn->setEnabled(false);
    statusLabel->setText("⏳ 翻译中，请稍候…");
    outputEdit->setPlainText("");

    // 创建子线程
    m_currentThread = new QThread();
    m_currentWorker = new TranslateWorker(
        text,
        srcLangBox->currentText(),
        dstLangBox->currentText(),
        promptEdit->toPlainText()
    );
    m_currentWorker->moveToThread(m_currentThread);

    // ✅ 用 QPointer 守护 this
    QPointer<PageTranslation> self = this;

    connect(m_currentThread, &QThread::started,
            m_currentWorker, &TranslateWorker::doTranslate);

    connect(m_currentWorker, &TranslateWorker::finished,
            this, [self](const QString &result) {
                if (!self) return;   // ✅ 对象已销毁，安全退出
                self->outputEdit->setPlainText(result);
                self->statusLabel->setText("✔ 翻译完成");
                self->translateBtn->setEnabled(true);
                self->clearBtn->setEnabled(true);
                self->m_currentThread  = nullptr;
                self->m_currentWorker  = nullptr;
            });

    connect(m_currentWorker, &TranslateWorker::errorOccurred,
            this, [self](const QString &err) {
                if (!self) return;   // ✅ 安全退出
                self->outputEdit->setPlainText("");
                self->statusLabel->setText("✘ " + err);
                self->translateBtn->setEnabled(true);
                self->clearBtn->setEnabled(true);
                self->m_currentThread  = nullptr;
                self->m_currentWorker  = nullptr;
            });

    connect(m_currentWorker, &TranslateWorker::finished,
            m_currentThread, &QThread::quit);
    connect(m_currentWorker, &TranslateWorker::errorOccurred,
            m_currentThread, &QThread::quit);
    connect(m_currentThread, &QThread::finished,
            m_currentWorker, &QObject::deleteLater);
    connect(m_currentThread, &QThread::finished,
            m_currentThread, &QObject::deleteLater);

    m_currentThread->start();
}
void PageTranslation::closeEvent(QCloseEvent *event) {
    if (m_currentThread && m_currentThread->isRunning()) {
        // 断开所有指向 this 的槽，防止回调已销毁的对象
        m_currentThread->disconnect(this);
        if (m_currentWorker) {
            m_currentWorker->disconnect(this);
        }

        // 等待线程自然结束（Python 调用无法强制中断，只能等）
        m_currentThread->quit();
        m_currentThread->wait();   // ⚠ 会短暂阻塞，但保证安全
    }
    QWidget::closeEvent(event);
}



void PageTranslation::onSwapLanguages() {
    int srcIdx = srcLangBox->currentIndex();
    int dstIdx = dstLangBox->currentIndex();
    srcLangBox->setCurrentIndex(dstIdx);
    dstLangBox->setCurrentIndex(srcIdx);

    // 同时互换文本内容
    QString srcText = inputEdit->toPlainText();
    QString dstText = outputEdit->toPlainText();
    inputEdit->setPlainText(dstText);
    outputEdit->setPlainText(srcText);
}

void PageTranslation::onClearClicked() {
    inputEdit->clear();
    outputEdit->clear();
    statusLabel->setText("就绪");
    charCountLabel->setText("0 / 5000 字符");
}

void PageTranslation::onCopyResult() {
    QString result = outputEdit->toPlainText();
    if (result.isEmpty()) {
        statusLabel->setText("⚠ 译文为空");
        return;
    }
    QApplication::clipboard()->setText(result);
    statusLabel->setText("✔ 已复制到剪贴板");
}

void PageTranslation::updateCharCount() {
    int count = inputEdit->toPlainText().length();
    charCountLabel->setText(
        QString("%1 / 5000 字符").arg(count)
    );
    if (count > 5000) {
        charCountLabel->setStyleSheet("color: #ff4444;");
    } else {
        charCountLabel->setStyleSheet("color: #888888;");
    }
}

// ── 样式 ─────────────────────────────────────────

void PageTranslation::applyStyles() {
    setStyleSheet(R"(
        #transTitleLabel {
            font-size: 18px;
            font-weight: bold;
            color: #a28ccb;
            font-family: 'Microsoft YaHei';
            padding-bottom: 4px;
            border-bottom: 2px solid #5ce1e6;
        }
        #transLangLabel {
            color: #555;
            font-size: 13px;
            font-family: 'Microsoft YaHei';
        }
        #transComboBox {
            border: 2px solid #a28ccb;
            border-radius: 4px;
            padding: 4px 8px;
            font-size: 13px;
            background: white;
            color: #333;
            min-height: 28px;
        }
        #transComboBox:focus {
            border-color: #5ce1e6;
        }
        #transSwapBtn {
            background-color: #a28ccb;
            color: white;
            border: none;
            border-radius: 18px;
            font-size: 18px;
            font-weight: bold;
        }
        #transSwapBtn:hover {
            background-color: #5ce1e6;
        }
        #transPanel {
            background: white;
            border: 2px solid #e0d4f5;
            border-radius: 6px;
            padding: 6px;
        }
        #transPanelHeader {
            font-size: 13px;
            font-weight: bold;
            color: #a28ccb;
            font-family: 'Microsoft YaHei';
            padding: 2px 4px;
        }
        #transInputEdit {
            border: 1px solid #d8ccf0;
            border-radius: 4px;
            font-size: 14px;
            color: #333;
            background: #fdfbff;
            font-family: 'Microsoft YaHei';
            padding: 6px;
        }
        #transInputEdit:focus {
            border-color: #a28ccb;
        }
        #transOutputEdit {
            border: 1px solid #c8f0f0;
            border-radius: 4px;
            font-size: 14px;
            color: #2a5c5c;
            background: #f5fffe;
            font-family: 'Microsoft YaHei';
            padding: 6px;
        }
        #transSplitter::handle {
            background-color: #d8f5f5;
            width: 6px;
        }
        #transCharCount {
            font-size: 11px;
            color: #888888;
            font-family: 'Microsoft YaHei';
        }
        #transStatusLabel {
            font-size: 12px;
            color: #666;
            font-family: 'Microsoft YaHei';
        }
        #transClearBtn {
            background-color: white;
            color: #a28ccb;
            border: 2px solid #a28ccb;
            border-radius: 4px;
            font-size: 13px;
            font-weight: bold;
            font-family: 'Microsoft YaHei';
        }
        #transClearBtn:hover {
            background-color: #fce4ec;
            border-color: #ff80ab;
            color: #ff80ab;
        }
        #transStartBtn {
            background-color: #a28ccb;
            color: white;
            border: none;
            border-radius: 4px;
            font-size: 13px;
            font-weight: bold;
            font-family: 'Microsoft YaHei';
        }
        #transStartBtn:hover {
            background-color: #5ce1e6;
        }
        #transStartBtn:disabled {
            background-color: #cccccc;
        }
        #transCopyBtn {
            background-color: transparent;
            color: #5c9e9e;
            border: 1px solid #5ce1e6;
            border-radius: 4px;
            font-size: 12px;
            font-family: 'Microsoft YaHei';
        }
        #transCopyBtn:hover {
            background-color: #e0fafa;
        }
        #transPromptPanel {
            background: #faf8ff;
            border: 1px dashed #c4b0e8;
            border-radius: 6px;
            padding: 6px;
        }
        #transPromptLabel {
            font-size: 12px;
            font-weight: bold;
            color: #9278bb;
            font-family: 'Microsoft YaHei';
        }
        #transPromptToggle {
            background: transparent;
            color: #a28ccb;
            border: 1px solid #a28ccb;
            border-radius: 4px;
            font-size: 12px;
            font-family: 'Microsoft YaHei';
        }
        #transPromptToggle:hover {
            background: #f0eaff;
        }
        #transPromptEdit {
            border: 1px solid #d4c4f0;
            border-radius: 4px;
            font-size: 13px;
            color: #555;
            background: #fdfbff;
            font-family: 'Microsoft YaHei';
            padding: 4px;
        }
        #transPromptEdit:focus {
            border-color: #a28ccb;
        }
    )");
}
