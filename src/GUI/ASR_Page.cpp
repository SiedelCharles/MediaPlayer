#include "ASR_Page.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QTreeWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QAction>
#include <QMenu>
#include <QSplitter>
#include <QMetaObject>
#include "WhisperAsr/WhisperAsr.hpp"
#include <chrono>
#include <thread>
#include <memory>

#include "sources/FileSource/FFmpegFileSource.hpp"
#include "VadFrame/WebRtcVadFrame.hpp"
#include "AudioTaskVad.hpp"
#include "AudioTaskPipeline.hpp"
#include "AudioTaskQueue.hpp"
#include "sink/WhisperQtSink.hpp"


const std::string model_path = "D:\\VisualStudio_Created\\VisualStudio_Resource\\audio\\whisper.cpp\\models\\ggml-small.bin";


PageTranscription::PageTranscription(QWidget *parent)
    : QWidget(parent)
{
    // 把信号转发到 slot_SetText（跨线程安全）
    // connect(this, &PageTranscription::Sig_ResultReady,
    //         this, &PageTranscription::slot_SetText,
    //         Qt::QueuedConnection);

    setupUi();
    setupMenu();
    applyStyles();
    
}

PageTranscription::~PageTranscription()
{
    stopWorker();
}

// ── 停止工作线程 ─────────────────────────────────────────
void PageTranscription::stopWorker()
{
    if (_pipeline) {
        _pipeline->stop();
    }
}

// ── UI搭建 ───────────────────────────────────────────────
void PageTranscription::setupUi()
{
    this->setObjectName("pageTranscription");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(10);

    // 顶部工具区
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->setSpacing(10);

    QLabel *titleLabel = new QLabel("■ 语音识别");
    titleLabel->setObjectName("pageTitleLabel");

    _btn_addFile = new QPushButton("添加文件");
    _btn_addFile->setObjectName("actionBtn");

    topLayout->addWidget(titleLabel);
    topLayout->addStretch();
    topLayout->addWidget(_btn_addFile);

    // 文件列表面板
    _file_list = new QWidget(this);
    _file_list->setObjectName("filePanel");

    QVBoxLayout *fileLayout = new QVBoxLayout(_file_list);
    fileLayout->setContentsMargins(10, 10, 10, 10);
    fileLayout->setSpacing(8);

    QLabel *fileLabel = new QLabel("文件列表");
    fileLabel->setObjectName("sectionLabel");

    _tree_files = new QTreeWidget(this);
    _tree_files->setObjectName("fileTree");
    _tree_files->setColumnCount(3);
    _tree_files->setHeaderLabels(QStringList() << "文件名" << "路径" << "状态");
    _tree_files->setRootIsDecorated(false);
    _tree_files->setAlternatingRowColors(true);
    _tree_files->setSelectionMode(QAbstractItemView::SingleSelection);

    _tree_files->header()->setStretchLastSection(false);
    _tree_files->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    _tree_files->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    _tree_files->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    fileLayout->addWidget(fileLabel);
    fileLayout->addWidget(_tree_files);

    // 结果面板
    QWidget *resultPanel = new QWidget(this);
    resultPanel->setObjectName("resultPanel");

    QVBoxLayout *resultLayout = new QVBoxLayout(resultPanel);
    resultLayout->setContentsMargins(10, 10, 10, 10);
    resultLayout->setSpacing(8);

    QLabel *resultLabel = new QLabel("识别结果");
    resultLabel->setObjectName("sectionLabel");

    _text_result = new QPlainTextEdit(this);
    _text_result->setObjectName("resultEdit");
    _text_result->setReadOnly(true);
    _text_result->setPlaceholderText("双击左侧文件后，识别结果将在这里显示...");

    resultLayout->addWidget(resultLabel);
    resultLayout->addWidget(_text_result);

    // 分割器
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(_file_list);
    splitter->addWidget(resultPanel);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 3);

    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(splitter, 1);

    // 连接
    connect(_btn_addFile, &QPushButton::clicked,
            this, &PageTranscription::slot_AddFile);

    connect(_tree_files, &QTreeWidget::itemDoubleClicked,
            this, &PageTranscription::slot_StartThread);
}

// ── 菜单 ──────────────────────────────────────────────────
void PageTranscription::setupMenu()
{
    _menu = new QMenuBar(this);
    _menu->setObjectName("pageMenuBar");

    QMenu *menuFile   = new QMenu("文件", _menu);
    QMenu *menuAction = new QMenu("操作", _menu);

    QAction *actAddFile    = new QAction("添加音频文件", this);
    QAction *actClearFiles = new QAction("清空文件列表", this);
    QAction *actClearText  = new QAction("清空识别结果", this);

    menuFile->addAction(actAddFile);
    menuFile->addSeparator();
    menuFile->addAction(actClearFiles);
    menuAction->addAction(actClearText);

    _menu->addMenu(menuFile);
    _menu->addMenu(menuAction);

    if (layout())
        static_cast<QVBoxLayout*>(layout())->setMenuBar(_menu);

    connect(actAddFile,    &QAction::triggered, this, &PageTranscription::slot_AddFile);
    connect(actClearFiles, &QAction::triggered, this, [this]() { _tree_files->clear(); });
    connect(actClearText,  &QAction::triggered, this, [this]() { _text_result->clear(); });
}

// ── 样式 ──────────────────────────────────────────────────
void PageTranscription::applyStyles()
{
    this->setStyleSheet(R"(
        #pageTranscription { background: transparent; }

        #pageMenuBar {
            background-color: #a28ccb;
            color: white;
            border: 2px solid #5ce1e6;
            border-bottom: 2px solid #5ce1e6;
            padding: 2px;
        }
        QMenuBar::item { background: transparent; color: white; padding: 6px 12px; }
        QMenuBar::item:selected { background: rgba(255,255,255,0.25); }

        QMenu { background-color: #fffafc; border: 2px solid #a28ccb; }
        QMenu::item { padding: 6px 20px; }
        QMenu::item:selected { background-color: #fce4ec; color: #ff80ab; }

        #pageTitleLabel { color: #a28ccb; font-size: 18px; font-weight: bold; padding: 4px 0; }
        #sectionLabel   { color: #a28ccb; font-size: 14px; font-weight: bold; }

        #actionBtn {
            background-color: rgba(255,255,255,0.7);
            border: 2px solid #a28ccb;
            color: #a28ccb;
            font-family: "Microsoft YaHei";
            font-weight: bold;
            font-size: 13px;
            padding: 8px 16px;
            border-radius: 4px;
        }
        #actionBtn:hover { background-color: #fce4ec; color: #ff80ab; border-color: #ff80ab; }

        #filePanel, #resultPanel {
            background-color: rgba(255,255,255,0.60);
            border: 2px solid #5ce1e6;
            border-radius: 6px;
        }

        #fileTree {
            background-color: rgba(255,255,255,0.90);
            border: 2px solid #a28ccb;
            border-radius: 4px;
            font-size: 13px;
            color: #444;
        }
        QHeaderView::section {
            background-color: #eadcf8; color: #7b68aa;
            padding: 6px; border: 1px solid #c8b6e2; font-weight: bold;
        }
        QTreeWidget::item          { height: 28px; }
        QTreeWidget::item:hover    { background-color: #fce4ec; }
        QTreeWidget::item:selected { background-color: #a28ccb; color: white; }

        #resultEdit {
            background-color: rgba(255,255,255,0.95);
            border: 2px solid #a28ccb;
            border-radius: 4px;
            padding: 8px;
            font-size: 13px;
            color: #333;
        }
        QSplitter::handle { background-color: #5ce1e6; width: 4px; }
    )");
}

// ── Slots ─────────────────────────────────────────────────

void PageTranscription::slot_AddFile()
{
    QStringList filePaths = QFileDialog::getOpenFileNames(
        this, "选择音频文件", "",
        "Audio Files (*.wav *.mp3 *.flac *.aac *.m4a);;All Files (*.*)"
    );

    if (filePaths.isEmpty()) return;

    for (const QString &path : filePaths) {
        QFileInfo info(path);

        bool alreadyExists = false;
        for (int i = 0; i < _tree_files->topLevelItemCount(); ++i) {
            if (_tree_files->topLevelItem(i)->text(1) == info.absoluteFilePath()) {
                alreadyExists = true;
                break;
            }
        }
        if (alreadyExists) continue;

        QTreeWidgetItem *item = new QTreeWidgetItem(_tree_files);
        item->setText(0, info.fileName());
        item->setText(1, info.absoluteFilePath());
        item->setText(2, "待处理");

        emit Sig_AddFile(path);
    }
}

void PageTranscription::slot_StartThread(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    if (!item) return;
    if (_current_item && _current_item != item) {
        _current_item->setText(2, "已停止");
    }
    // 停止上一个任务
    stopWorker();

    _current_item = item;
    _current_item->setText(2, "处理中...");
    _text_result->clear();

    _pipeline = std::make_unique<audiotask::core::AudioTaskPipeline>();

    const std::string filePath = item->text(1).toStdString();
    std::shared_ptr<audiotask::source::FileSource> source = std::make_shared<audiotask::source::FFmpegFileSource>();
    audiotask::source::SourceFormat out_fmt;
    out_fmt.channels = 1;
    out_fmt.sample_rate = 16000;
    out_fmt.format = audiotask::source::SampleFormat::S16;
    source->set_output_format(out_fmt);
    source->init(filePath);

    std::unique_ptr<audiotask::vad::VadFrame> webrtc_frame = std::make_unique<audiotask::vad::WebRtcVadFrame>();
    std::shared_ptr<audiotask::vad::AudioTaskVad> vad = std::make_shared<audiotask::vad::AudioTaskVad>(std::move(webrtc_frame));

    
    std::shared_ptr<audiotask::asr::WhisperAsr> asr = std::make_shared<audiotask::asr::WhisperAsr>();
    whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    params.print_realtime   = false;
    params.print_progress   = false;
    params.print_timestamps = false;
    params.print_special    = false;
    params.translate        = false;
    params.language         = "ja";
    params.n_threads        = 4;
    params.offset_ms        = 0;
    params.duration_ms      = 0;
    params.single_segment   = true;
    std::string prompt = "これは日本語の成人向けASMR音声です。登場人物はマンドラベーゼ、キュラピンク、ポルルン。魔法少女、妖精、悪の幹部、プリンプリンクリスタル。触手、種付け、性転換、産卵、潮吹き、苗床。";
    params.initial_prompt = prompt.data();
    asr->init(model_path, params);

    uint64_t capacity = 1ull << 30;
    std::shared_ptr<audiotask::core::AudioTaskQueue> buffer_stream = std::make_shared<audiotask::core::AudioTaskQueue>(capacity);

    std::shared_ptr<audiotask::sink::AudioTaskAsrSink> sink = std::make_unique<audiotask::sink::AudioTaskAsrSink>();
    _pipeline->add(source);
    // _pipeline->add(vad);
    _pipeline->add(asr);
    _pipeline->add(buffer_stream);
    _pipeline->add(sink);
    
    _pipeline->link(source.get(), buffer_stream.get());
    // _pipeline->link(source.get(), vad.get());
    // _pipeline->link(vad.get(), buffer_stream.get());
    _pipeline->link(buffer_stream.get(), asr.get());
    _pipeline->link(asr.get(), sink.get());
    connect(sink.get(), &audiotask::sink::AudioTaskAsrSink::text_recognized, this, &PageTranscription::slot_SetText);
    auto is_start = _pipeline->start();
    connect(sink.get(), &audiotask::sink::AudioTaskAsrSink::text_done, this, &PageTranscription::slot_Text_Done);
}

void PageTranscription::slot_SetText(const size_t vad_cnt, const QString &text)
{
    _text_result->appendPlainText(text+"\n");
}

void PageTranscription::slot_Text_Done()
{
    _current_item->setText(2, "处理结束...");
    _pipeline->stop();
}
