#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSplitter>
#include <QThread>
#include <QFutureWatcher>
// #include <QtConcurrent>


// 前向声明翻译Worker
class TranslateWorker : public QObject {
    Q_OBJECT
public:
    explicit TranslateWorker(const QString &text,
                             const QString &srcLang,
                             const QString &dstLang,
                             const QString &prompt,
                             QObject *parent = nullptr);
signals:
    void finished(const QString &result);
    void errorOccurred(const QString &errMsg);

public slots:
    void doTranslate();

private:
    QString m_text;
    QString m_srcLang;
    QString m_dstLang;
    QString m_prompt;
};

class PageTranslation : public QWidget {
    Q_OBJECT
public:
    explicit PageTranslation(QWidget *parent = nullptr);

private slots:
    void onTranslateClicked();
    void onSwapLanguages();
    void onClearClicked();
    void onCopyResult();

    
protected:
    void closeEvent(QCloseEvent *event) override;


private:
    QComboBox   *srcLangBox;
    QComboBox   *dstLangBox;
    QTextEdit   *inputEdit;
    QTextEdit   *outputEdit;
    QPushButton *translateBtn;
    QPushButton *swapBtn;
    QPushButton *clearBtn;
    QPushButton *copyBtn;
    QLabel      *charCountLabel;
    QLabel      *statusLabel;
    QTextEdit   *promptEdit;

    QThread*  m_currentThread  = nullptr;
    TranslateWorker* m_currentWorker = nullptr;

    void setupUi();
    void applyStyles();
    void updateCharCount();
};
