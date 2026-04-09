#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QSplitter>

// 前向声明 TTS 工作类
class TTSWorker : public QObject {
    Q_OBJECT
public:
    explicit TTSWorker(const QString &subtitlePath,
                       const QString &outputDir,
                       const QString &samplePath,
                       const QString &prompt,
                       QObject *parent = nullptr);

signals:
    void finished(const QString &outputAudioPath);  // 合成成功，返回生成的音频路径
    void errorOccurred(const QString &errMsg);
    void progress(int percent);

public slots:
    void doSynthesize();

private:
    QString m_subtitlePath;
    QString m_outputDir;
    QString m_samplePath;
    QString m_prompt;
};

class PageTTS : public QWidget {
    Q_OBJECT
public:
    explicit PageTTS(QWidget *parent = nullptr);
    ~PageTTS();

private slots:
    void onSubtitleBrowseClicked();
    void onSavePathBrowseClicked();
    void onSampleBrowseClicked();
    void onMergeBrowseClicked();
    void onGenerateClicked();
    void onMergeAudioClicked();

    void onSynthesisFinished(const QString &audioPath);
    void onSynthesisError(const QString &errMsg);
    void onSynthesisProgress(int percent);
protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void setupUi();
    void applyStyles();
    void updateButtonState(bool synthesizing);

    // UI 组件
    QLineEdit   *subtitleEdit;
    QPushButton *subtitleBtn;
    QLineEdit   *savePathEdit;
    QPushButton *savePathBtn;
    QLineEdit   *sampleEdit;
    QPushButton *sampleBtn;
    QTextEdit   *promptEdit;
    QLineEdit   *mergeEdit;
    QPushButton *mergeBtn;
    QPushButton *generateBtn;
    QPushButton *mergeAudioBtn;
    QLabel      *statusLabel;

    QString      m_subtitlePath;
    QString      m_outputDir;
    QString      m_samplePath;
    QString      m_prompt;
    QString      m_mergePath;  // 待合并的音频文件路径
    // 状态
    QString      m_lastGeneratedAudio;  // 最近一次生成的音频文件路径
    QThread      *m_workerThread = nullptr;
    TTSWorker    *m_worker = nullptr;
};