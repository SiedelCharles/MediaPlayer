#ifndef FILELISTTHREAD_H
#define FILELISTTHREAD_H
#include <QThread>
#include <QListWidget>

class FileListThread : public QThread
{
    Q_OBJECT
public:
    FileListThread(const QString& src_path, QObject *parent = nullptr, QListWidget *list = nullptr);
private:
    QWidget *_list;
    QString _src_path;

    void OpenFiles(const QString &path);
};

#endif // FILELISTTHREAD_H
