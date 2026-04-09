#pragma push_macro("slots")
#undef slots
#include "TTS/synthesis_script.h"
#pragma pop_macro("slots")
#include "TTS_Page.hpp"
#include <QFileDialog>
#include <QClipboard>
#include <QApplication>
#include <QCoreApplication>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QThread>
#include <QPointer>
#include "utils/srttimestamp.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <vector>
#include <string>
#include <QGroupBox>
#include <iostream>
#include <fstream>

struct SubtitleEntry {
    TimeStampPair times;
    QString text;
};

std::vector<SubtitleEntry> parseSubtitleFileToEntries(const QString &path) {
    std::vector<SubtitleEntry> entries;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return entries;

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    QString content = stream.readAll();
    file.close();

    QStringList blocks = content.split(QRegularExpression("\n\\s*\n"), Qt::SkipEmptyParts);
    QRegularExpression timeRegex(
        R"((\d{2}):(\d{2}):(\d{2})[.,](\d{3})\s*-->\s*(\d{2}):(\d{2}):(\d{2})[.,](\d{3}))");

    for (const QString &block : blocks) {
        QStringList lines = block.split('\n', Qt::SkipEmptyParts);
        if (lines.size() < 2) continue;

        int idx = 0;
        bool isNum = false;
        lines[0].toInt(&isNum);
        if (isNum) idx = 1;
        if (idx >= lines.size()) continue;

        QRegularExpressionMatch match = timeRegex.match(lines[idx]);
        if (!match.hasMatch()) continue;

        auto toMs = [](int h, int m, int s, int ms) -> qint64 {
            return h * 3600000LL + m * 60000LL + s * 1000LL + ms;
        };
        qint64 startMs = toMs(match.captured(1).toInt(), match.captured(2).toInt(),
                               match.captured(3).toInt(), match.captured(4).toInt());
        qint64 endMs   = toMs(match.captured(5).toInt(), match.captured(6).toInt(),
                               match.captured(7).toInt(), match.captured(8).toInt());

        QStringList textLines = lines.mid(idx + 1);
        QString text = textLines.join(' ').trimmed();
        if (text.isEmpty()) continue;

        entries.push_back({TimeStampPair(startMs, endMs), text});
    }
    return entries;
}

// ========== Python 解释器管理 ==========
static py::scoped_interpreter* g_guard = nullptr;

// static void ensurePythonInit() {
//     qDebug() << "=== 开始初始化 Python ===";
//     qDebug() << "当前线程:" << QThread::currentThread();
//     qDebug() << "主线程:" << QCoreApplication::instance()->thread();
    
//     Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());
    
//     if (!g_guard) {
//         qDebug() << "→ 设置 Python Home...";
//         Py_SetPythonHome(
//             L"D:/VisualStudio_Created/VisualStudio_Resource/"
//             L"miniconda/envs/audio"
//         );
//         qDebug() << "✓ Python Home 设置完成";
        
//         qDebug() << "→ 创建 scoped_interpreter...";
//         g_guard = new py::scoped_interpreter();  // ← 很可能卡在这里
//         qDebug() << "✓ scoped_interpreter 创建完成";
        
//         qDebug() << "→ 释放 GIL...";
//         PyEval_SaveThread();
//         qDebug() << "✓ GIL 释放完成";
//     }
//     qDebug() << "=== Python 初始化完成 ===";
// }

// ========== TTSWorker ==========
TTSWorker::TTSWorker(const QString &subtitlePath,
                     const QString &outputDir,
                     const QString &samplePath,
                     const QString &prompt,
                     QObject *parent)
    : QObject(parent)
    , m_subtitlePath(subtitlePath)
    , m_outputDir(outputDir)
    , m_samplePath(samplePath)
    , m_prompt(prompt)
{}

void TTSWorker::doSynthesize() {
    try {
        QString resultDir;
        {
            // ✅ 显式获取 GIL
            py::gil_scoped_acquire gil;

            py::module_ sys = py::module_::import("sys");
            std::string scriptDir =
                "D:/VisualStudio_Created/VisualStudio_Project/"
                "Projects/MediaPlayer/src/TTS";
            sys.attr("path").attr("append")(scriptDir);

            qDebug() << "Python sys.path:" << QString::fromStdString(py::str(sys.attr("path")));

            py::module_ tts_module = py::module_::import("synthesis_script");
            // py::object upload_func     = tts_module.attr("upload_custom_voice");
            // py::object synthesize_func = tts_module.attr("synthesize_with_custom_voice");
            std::string voice_name = "Voice";
            std::string voice_id = upload_custom_voice(tts_module, m_samplePath.toStdString(), voice_name);

            if (voice_id.empty()) {
                emit errorOccurred("TTS音色上传失败");
                return;
            }
            qDebug() << "上传音色成功，voice_id:" << QString::fromStdString(voice_id);
            // 2. 解析字幕
            auto entries = parseSubtitleFileToEntries(m_subtitlePath);
            if (entries.empty()) {
                emit errorOccurred("字幕文件无效或无有效条目");
                return;
            }

            // 3. 创建输出目录
            QDir dir(m_outputDir);
            if (!dir.exists() && !dir.mkpath(".")) {
                emit errorOccurred("无法创建输出目录");
                return;
            }

            // // 4. 准备时间戳文件
            // QString timestampFilePath = dir.filePath("timestamps.txt");
            // std::ofstream tsFile(timestampFilePath.toStdString());
            // if (!tsFile.is_open()) {
            //     emit errorOccurred("无法创建时间戳文件");
            //     return;
            // }
            // tsFile << "# 格式: 音频文件名 开始时间(ms) 结束时间(ms) 文本\n";
            QString timestampFilePath = dir.filePath("timestamps.txt");
            QFile tsFile(timestampFilePath);
            if (!tsFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                emit errorOccurred("无法创建时间戳文件: " + tsFile.errorString());
                return;
            }
            QTextStream out(&tsFile);
            out << "# 格式: 音频文件名 开始时间(ms) 结束时间(ms) 文本\n";

            // 5. 逐句合成
            int total = static_cast<int>(entries.size());
            for (int i = 0; i < total; ++i) {
                const auto &entry = entries[i];
                if (entry.text.isEmpty()) continue;

                QString audioName = QString("subtitle_%1.wav").arg(i + 1, 4, 10, QChar('0'));
                QString audioPath = dir.filePath(audioName);

                synthesize_with_custom_voice(
                    tts_module,
                    voice_id,
                    entry.text.toStdString(),
                    audioPath.toStdString(),
                    m_prompt.toStdString(),
                    24000,
                    0.7
                );

                int64_t startMs = entry.times.timestamp1.milliseconds();
                int64_t endMs   = entry.times.timestamp2.milliseconds();
                out << audioName << " "
                    << startMs << " "
                    << endMs   << " "
                    << entry.text << "\n";

                int percent = 15 + (i + 1) * 80 / total;
            }
            tsFile.close();
            resultDir = m_outputDir;
        }
        emit finished(resultDir);

    } catch (const py::error_already_set &e) {
        emit errorOccurred(QString("Python错误: %1").arg(e.what()));
    } catch (const std::exception &e) {
        emit errorOccurred(QString("TTS合成错误: %1").arg(e.what()));
    } catch (...) {
        emit errorOccurred("TTS合成发生未知错误");
    }
}

// ========== PageTTS ==========
PageTTS::PageTTS(QWidget *parent)
    : QWidget(parent)
{
    // ensurePythonInit();
    setupUi();
    applyStyles();
    connect(subtitleBtn, &QPushButton::clicked, this, &PageTTS::onSubtitleBrowseClicked);
    connect(savePathBtn, &QPushButton::clicked, this, &PageTTS::onSavePathBrowseClicked);
    connect(sampleBtn, &QPushButton::clicked, this, &PageTTS::onSampleBrowseClicked);
    connect(mergeBtn, &QPushButton::clicked, this, &PageTTS::onMergeBrowseClicked);
    connect(generateBtn, &QPushButton::clicked, this, &PageTTS::onGenerateClicked);
    connect(mergeAudioBtn, &QPushButton::clicked, this, &PageTTS::onMergeAudioClicked);
}

PageTTS::~PageTTS() {
    // 等待工作线程安全退出
    if (m_workerThread && m_workerThread->isRunning()) {
        m_workerThread->quit();
        m_workerThread->wait(5000);
    }
}

void PageTTS::onSubtitleBrowseClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("选择字幕文件"),
        QDir::homePath(),
        tr("字幕文件 (*.srt *.vtt *.txt);;所有文件 (*)"));

    if (fileName.isEmpty())
        return;

    m_subtitlePath = fileName;

    QString displayName = QFileInfo(fileName).fileName();
    subtitleEdit->setText(displayName);
    subtitleEdit->setToolTip(fileName);

    statusLabel->setText(tr("已选择字幕文件: %1").arg(displayName));
}

void PageTTS::onSavePathBrowseClicked() {
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("选择输出目录"),
        QDir::homePath());

    if (dir.isEmpty())
        return;

    m_outputDir = dir;

    QString displayName = QFileInfo(dir).fileName();
    if (displayName.isEmpty())
        displayName = dir;

    savePathEdit->setText(displayName);
    savePathEdit->setToolTip(dir);

    statusLabel->setText(tr("已选择输出目录: %1").arg(displayName));
}

void PageTTS::onSampleBrowseClicked() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("选择参考音频"),
        QDir::homePath(),
        tr("音频文件 (*.mp3 *.wav *.flac *.m4a);;所有文件 (*)"));

    if (fileName.isEmpty())
        return;

    m_samplePath = fileName;

    QString displayName = QFileInfo(fileName).fileName();
    sampleEdit->setText(displayName);
    sampleEdit->setToolTip(fileName);

    statusLabel->setText(tr("已选择参考音频: %1").arg(displayName));
}

void PageTTS::onMergeBrowseClicked() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("选择背景音频"),
        QDir::homePath(),
        tr("音频文件 (*.mp3 *.wav *.flac *.m4a);;所有文件 (*)"));

    if (fileName.isEmpty())
        return;

    m_mergePath = fileName;

    QString displayName = QFileInfo(fileName).fileName();
    mergeEdit->setText(displayName);
    mergeEdit->setToolTip(fileName);

    statusLabel->setText(tr("已选择背景音频: %1").arg(displayName));
}

void PageTTS::closeEvent(QCloseEvent *event) {
    if (m_workerThread && m_workerThread->isRunning()) {
        // 断开所有指向 this 的槽，防止回调已销毁的对象
        m_workerThread->disconnect(this);
        if (m_worker) {
            m_worker->disconnect(this);
        }

        // 等待线程自然结束
        m_workerThread->quit();
        m_workerThread->wait();
    }
    QWidget::closeEvent(event);
}

void PageTTS::onGenerateClicked() {
    if (m_workerThread && m_workerThread->isRunning()) {
        statusLabel->setText("⚠ 正在合成中，请等待完成");
        return;
    }

    QString subtitlePath = m_subtitlePath.trimmed();
    if (subtitlePath.isEmpty()) {
        statusLabel->setText("⚠ 请先选择字幕文件");
        return;
    }
    QString outputDir = m_outputDir.trimmed();
    if (outputDir.isEmpty()) {
        statusLabel->setText("⚠ 请先选择保存路径");
        return;
    }
    QDir dir(outputDir);
    if (!dir.exists() && !dir.mkpath(".")) {
        statusLabel->setText("⚠ 无法创建输出目录");
        return;
    }

    updateButtonState(true);
    statusLabel->setText("⏳ 正在合成语音，请稍候…");
    m_lastGeneratedAudio.clear();

    m_workerThread = new QThread(this);
    m_worker = new TTSWorker(
        subtitlePath,
        outputDir,
        m_samplePath,
        promptEdit->toPlainText().trimmed()
    );
    m_worker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::started,       m_worker,       &TTSWorker::doSynthesize);
    connect(m_worker,       &TTSWorker::finished,    this,            &PageTTS::onSynthesisFinished);
    connect(m_worker,       &TTSWorker::errorOccurred,this,           &PageTTS::onSynthesisError);
    connect(m_worker,       &TTSWorker::progress,    this,            &PageTTS::onSynthesisProgress);

    // 结束后清理
    connect(m_worker, &TTSWorker::finished,
            m_workerThread, &QThread::quit);
    connect(m_worker, &TTSWorker::errorOccurred,
            m_workerThread, &QThread::quit);
    connect(m_workerThread, &QThread::finished,
            m_worker, &QObject::deleteLater);
    connect(m_workerThread, &QThread::finished,
            m_workerThread, &QObject::deleteLater);
    // 线程销毁后重置指针
    connect(m_workerThread, &QThread::finished, this, [this]() {
        m_workerThread = nullptr;
        m_worker       = nullptr;
    });

    m_workerThread->start();
    qDebug() << "TTS synthesis started in worker thread.";
}

void PageTTS::onMergeAudioClicked() {
    if (m_lastGeneratedAudio.isEmpty() || !QFile::exists(m_lastGeneratedAudio)) {
        statusLabel->setText("⚠ 没有可合并的音频，请先生成");
        return;
    }
    // mergeEdit 存的是显示名，实际路径存在 m_mergePath
    if (m_mergePath.isEmpty() || !QFile::exists(m_mergePath)) {
        statusLabel->setText("⚠ 请先选择要合并的音频文件");
        return;
    }

    statusLabel->setText("⏳ 正在合并音频…");
    QTimer::singleShot(1000, this, [this]() {
        statusLabel->setText("✔ 合并完成");
    });
}

void PageTTS::onSynthesisFinished(const QString &audioPath) {
    m_lastGeneratedAudio = audioPath;
    statusLabel->setText(QString("✔ 合成完成: %1").arg(QFileInfo(audioPath).fileName()));
    updateButtonState(false);
    QMessageBox::information(this, "成功", "语音合成完成！");
}

void PageTTS::onSynthesisError(const QString &errMsg) {
    statusLabel->setText("✘ " + errMsg);
    updateButtonState(false);
    QMessageBox::critical(this, "错误", errMsg);
}

void PageTTS::onSynthesisProgress(int percent) {
    statusLabel->setText(QString("合成进度: %1%").arg(percent));
}

void PageTTS::updateButtonState(bool synthesizing) {
    generateBtn->setEnabled(!synthesizing);
    mergeAudioBtn->setEnabled(!synthesizing);
    subtitleBtn->setEnabled(!synthesizing);
    savePathBtn->setEnabled(!synthesizing);
    sampleBtn->setEnabled(!synthesizing);
    mergeBtn->setEnabled(!synthesizing);
}

void PageTTS::setupUi() {
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(20);

    // ========== 标题区域 ==========
    QLabel *titleLabel = new QLabel("语音合成工作台");
    titleLabel->setObjectName("ttsTitle");
    titleLabel->setAlignment(Qt::AlignLeft);
    mainLayout->addWidget(titleLabel);

    // ========== 主内容区域 ==========
    QWidget *contentWidget = new QWidget();
    contentWidget->setObjectName("ttsContentWidget");
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(20);
    contentLayout->setContentsMargins(24, 24, 24, 24);

    // ========== 1. 输入配置区域 ==========
    QGroupBox *inputGroup = new QGroupBox("输入配置");
    inputGroup->setObjectName("ttsGroupBox");
    QVBoxLayout *inputLayout = new QVBoxLayout(inputGroup);
    inputLayout->setSpacing(16);

    // 字幕文件行
    QHBoxLayout *subtitleRow = new QHBoxLayout();
    subtitleRow->setSpacing(12);

    QLabel *subtitleIcon = new QLabel("📄");
    subtitleIcon->setFixedWidth(24);

    QLabel *subtitleLabel = new QLabel("字幕文件");
    subtitleLabel->setObjectName("ttsFieldLabel");
    subtitleLabel->setFixedWidth(80);

    subtitleEdit = new QLineEdit();
    subtitleEdit->setPlaceholderText("选择 SRT 或 TXT 字幕文件");
    subtitleEdit->setObjectName("ttsLineEdit");
    subtitleEdit->setReadOnly(true);
    subtitleEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    subtitleBtn = new QPushButton("选择文件");
    subtitleBtn->setObjectName("ttsBrowseBtn");
    subtitleBtn->setFixedWidth(100);

    subtitleRow->addWidget(subtitleIcon);
    subtitleRow->addWidget(subtitleLabel);
    subtitleRow->addWidget(subtitleEdit, 1);
    subtitleRow->addWidget(subtitleBtn);
    inputLayout->addLayout(subtitleRow);

    // 输出目录行
    QHBoxLayout *outputRow = new QHBoxLayout();
    outputRow->setSpacing(12);

    QLabel *outputIcon = new QLabel("📁");
    outputIcon->setFixedWidth(24);

    QLabel *saveLabel = new QLabel("输出目录");
    saveLabel->setObjectName("ttsFieldLabel");
    saveLabel->setFixedWidth(80);

    savePathEdit = new QLineEdit();
    savePathEdit->setPlaceholderText("选择音频文件保存位置");
    savePathEdit->setObjectName("ttsLineEdit");
    savePathEdit->setReadOnly(true);
    savePathEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    savePathBtn = new QPushButton("选择目录");
    savePathBtn->setObjectName("ttsBrowseBtn");
    savePathBtn->setFixedWidth(100);

    outputRow->addWidget(outputIcon);
    outputRow->addWidget(saveLabel);
    outputRow->addWidget(savePathEdit, 1);
    outputRow->addWidget(savePathBtn);
    inputLayout->addLayout(outputRow);

    contentLayout->addWidget(inputGroup);

    // ========== 2. 音色配置区域 ==========
    QGroupBox *voiceGroup = new QGroupBox("音色配置");
    voiceGroup->setObjectName("ttsGroupBox");
    QVBoxLayout *voiceLayout = new QVBoxLayout(voiceGroup);
    voiceLayout->setSpacing(16);

    // 音频样本行
    QHBoxLayout *sampleRow = new QHBoxLayout();
    sampleRow->setSpacing(12);

    QLabel *sampleIcon = new QLabel("🎤");
    sampleIcon->setFixedWidth(24);

    QLabel *sampleLabel = new QLabel("参考音频");
    sampleLabel->setObjectName("ttsFieldLabel");
    sampleLabel->setFixedWidth(80);

    sampleEdit = new QLineEdit();
    sampleEdit->setPlaceholderText("选择用于音色克隆的音频样本");
    sampleEdit->setObjectName("ttsLineEdit");
    sampleEdit->setReadOnly(true);
    sampleEdit->setMinimumWidth(0);
    sampleEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    sampleBtn = new QPushButton("选择音频");
    sampleBtn->setObjectName("ttsBrowseBtn");
    sampleBtn->setFixedWidth(100);

    sampleRow->addWidget(sampleIcon);
    sampleRow->addWidget(sampleLabel);
    sampleRow->addWidget(sampleEdit, 1);
    sampleRow->addWidget(sampleBtn);
    voiceLayout->addLayout(sampleRow);

    // 提示词区域
    QHBoxLayout *promptRow = new QHBoxLayout();
    promptRow->setSpacing(12);

    QLabel *promptIcon = new QLabel("✏️");
    promptIcon->setFixedWidth(24);
    promptIcon->setAlignment(Qt::AlignTop);

    QLabel *promptLabel = new QLabel("合成提示");
    promptLabel->setObjectName("ttsFieldLabel");
    promptLabel->setFixedWidth(80);
    promptLabel->setAlignment(Qt::AlignTop);

    promptEdit = new QTextEdit();
    promptEdit->setPlaceholderText("描述期望的语音风格，例如：温柔语气、慢速朗读、情感饱满等");
    promptEdit->setObjectName("ttsPromptEdit");
    promptEdit->setMaximumHeight(80);
    promptEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    promptRow->addWidget(promptIcon);
    promptRow->addWidget(promptLabel);
    promptRow->addWidget(promptEdit, 1);
    voiceLayout->addLayout(promptRow);

    contentLayout->addWidget(voiceGroup);

    // ========== 3. 音频合并区域 ==========
    QGroupBox *mergeGroup = new QGroupBox("音频合并（可选）");
    mergeGroup->setObjectName("ttsGroupBox");
    QVBoxLayout *mergeLayout = new QVBoxLayout(mergeGroup);
    mergeLayout->setSpacing(16);

    // 合并音频行
    QHBoxLayout *mergeRow = new QHBoxLayout();
    mergeRow->setSpacing(12);

    QLabel *mergeIcon = new QLabel("🔗");
    mergeIcon->setFixedWidth(24);

    QLabel *mergeLabel = new QLabel("背景音频");
    mergeLabel->setObjectName("ttsFieldLabel");
    mergeLabel->setFixedWidth(80);

    mergeEdit = new QLineEdit();
    mergeEdit->setPlaceholderText("选择要与生成音频合并的背景音频");
    mergeEdit->setObjectName("ttsLineEdit");
    mergeEdit->setReadOnly(true);
    mergeEdit->setMinimumWidth(0);
    mergeEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    mergeBtn = new QPushButton("选择音频");
    mergeBtn->setObjectName("ttsBrowseBtn");
    mergeBtn->setFixedWidth(100);

    mergeRow->addWidget(mergeIcon);
    mergeRow->addWidget(mergeLabel);
    mergeRow->addWidget(mergeEdit, 1);
    mergeRow->addWidget(mergeBtn);
    mergeLayout->addLayout(mergeRow);

    contentLayout->addWidget(mergeGroup);

    mainLayout->addWidget(contentWidget);

    // ========== 底部操作区域 ==========
    QWidget *bottomWidget = new QWidget();
    bottomWidget->setObjectName("ttsBottomWidget");
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomWidget);
    bottomLayout->setContentsMargins(24, 16, 24, 16);
    bottomLayout->setSpacing(12);

    // 状态标签
    statusLabel = new QLabel("就绪");
    statusLabel->setObjectName("ttsStatusLabel");
    statusLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    bottomLayout->addWidget(statusLabel, 1);
    bottomLayout->addStretch();

    // 合并按钮
    mergeAudioBtn = new QPushButton("合并音频");
    mergeAudioBtn->setObjectName("ttsSecondaryBtn");
    mergeAudioBtn->setFixedHeight(40);
    mergeAudioBtn->setFixedWidth(120);
    bottomLayout->addWidget(mergeAudioBtn);

    // 生成按钮
    generateBtn = new QPushButton("开始生成");
    generateBtn->setObjectName("ttsGenerateBtn");
    generateBtn->setFixedHeight(40);
    generateBtn->setFixedWidth(120);
    bottomLayout->addWidget(generateBtn);

    mainLayout->addWidget(bottomWidget);
    mainLayout->addStretch();

}

void PageTTS::applyStyles() {
    QString style = R"(
        /* 主背景 */
        PageTTS {
            background-color: #f5f7fa;
        }

        /* 标题 */
        #ttsTitle {
            color: #2c3e50;
            font-family: 'Microsoft YaHei';
            font-weight: bold;
            font-size: 24px;
            padding-bottom: 8px;
        }

        /* 内容区域 */
        #ttsContentWidget {
            background-color: white;
            border-radius: 12px;
            border: 1px solid #e1e8ed;
        }

        /* 分组框 */
        #ttsGroupBox {
            font-family: 'Microsoft YaHei';
            font-size: 14px;
            font-weight: bold;
            color: #34495e;
            border: 2px solid #e8eef3;
            border-radius: 8px;
            padding: 16px;
            margin-top: 8px;
        }
        #ttsGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 8px;
            background-color: white;
        }

        /* 字段标签 */
        #ttsFieldLabel {
            color: #5a6c7d;
            font-family: 'Microsoft YaHei';
            font-weight: 600;
            font-size: 13px;
        }

        /* 输入框 */
        #ttsLineEdit {
            border: 2px solid #dfe6e9;
            border-radius: 6px;
            padding: 8px 12px;
            font-size: 13px;
            font-family: 'Microsoft YaHei';
            background-color: #f8f9fa;
            color: #2d3436;
        }
        #ttsLineEdit:focus {
            border-color: #74b9ff;
            background-color: white;
        }
        #ttsLineEdit:hover {
            border-color: #b2bec3;
        }

        /* 浏览按钮 */
        #ttsBrowseBtn {
            background-color: #6c5ce7;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-family: 'Microsoft YaHei';
            font-weight: 600;
            font-size: 13px;
        }
        #ttsBrowseBtn:hover {
            background-color: #5f4dd1;
        }
        #ttsBrowseBtn:pressed {
            background-color: #4d3fb8;
        }

        /* 多行提示词输入框 */
        #ttsPromptEdit {
            border: 2px solid #dfe6e9;
            border-radius: 6px;
            padding: 10px;
            font-size: 13px;
            font-family: 'Microsoft YaHei';
            background-color: #f8f9fa;
            color: #2d3436;
            line-height: 1.5;
        }
        #ttsPromptEdit:focus {
            border-color: #74b9ff;
            background-color: white;
        }
        #ttsPromptEdit:hover {
            border-color: #b2bec3;
        }

        /* 底部区域 */
        #ttsBottomWidget {
            background-color: white;
            border-radius: 12px;
            border: 1px solid #e1e8ed;
        }

        /* 主生成按钮 */
        #ttsGenerateBtn {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #00b894, stop:1 #00a383);
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 14px;
            font-weight: bold;
            font-family: 'Microsoft YaHei';
            padding: 10px 24px;
        }
        #ttsGenerateBtn:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #00d9a5, stop:1 #00b894);
        }
        #ttsGenerateBtn:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #009975, stop:1 #008866);
        }
        #ttsGenerateBtn:disabled {
            background: #b2bec3;
        }

        /* 次要按钮（合并） */
        #ttsSecondaryBtn {
            background-color: #74b9ff;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 14px;
            font-weight: bold;
            font-family: 'Microsoft YaHei';
            padding: 10px 24px;
        }
        #ttsSecondaryBtn:hover {
            background-color: #5fa3ff;
        }
        #ttsSecondaryBtn:pressed {
            background-color: #4a8eeb;
        }
        #ttsSecondaryBtn:disabled {
            background: #b2bec3;
        }

        /* 状态标签 */
        #ttsStatusLabel {
            font-size: 13px;
            color: #636e72;
            font-family: 'Microsoft YaHei';
            font-weight: 500;
            padding: 8px 12px;
            background-color: #f8f9fa;
            border-radius: 6px;
        }
    )";
    this->setStyleSheet(style);
}