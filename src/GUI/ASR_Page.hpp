#pragma once

#include <QWidget>
#include <QMenuBar>
#include <QTreeWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <memory>
#include <thread>
#include <atomic>

#include "core/AudioTaskPipeline.hpp"

class QTreeWidget;
class QPlainTextEdit;
class QPushButton;
class QLabel;
class QTreeWidgetItem;


class PageTranscription : public QWidget
{
    Q_OBJECT

signals:
    void Sig_AddFile(const QString path);
    void Sig_ResultReady(const QString text);

public:
    explicit PageTranscription(QWidget *parent = nullptr);
    ~PageTranscription();

public slots:
    void slot_AddFile();
    void slot_StartThread(QTreeWidgetItem *item, int column);
    void slot_SetText(const size_t vad_cnt, const QString& text);
    void slot_Text_Done();

private:
    void setupUi();
    void setupMenu();
    void applyStyles();
    void stopWorker();

private:
    QMenuBar        *_menu        = nullptr;
    QWidget         *_file_list   = nullptr;
    QTreeWidget     *_tree_files  = nullptr;
    QPlainTextEdit  *_text_result = nullptr;
    QPushButton     *_btn_addFile = nullptr;

    // 当前正在处理的 item
    QTreeWidgetItem *_current_item = nullptr;
    std::unique_ptr<audiotask::core::AudioTaskPipeline> _pipeline;
    std::atomic<bool> _running{false};
};

