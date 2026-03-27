#ifndef FILETREETHREAD_H
#define FILETREETHREAD_H
#include <QThread>
#include <QTreeWidget>

class FileTreeThread : public QThread
{
    Q_OBJECT
signals:
    void SigOpenFileFinished();
public:
    FileTreeThread(const QString& src_path, QObject *parent = nullptr, QTreeWidget *list = nullptr);
    ~FileTreeThread();
protected:
    virtual void run() override;
private:
    void OpenFiles(const QString &path);

    QWidget *_list;
    QString _src_path;
    QTreeWidgetItem *_root;
};

#endif // FILETREETHREAD_H
