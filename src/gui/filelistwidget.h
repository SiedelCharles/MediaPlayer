#ifndef FILELISTWIDGET_H
#define FILELISTWIDGET_H

#include <QListWidget>
#include "filelistthread.h"

class FileListWidget : public QListWidget
{
public:
    FileListWidget(QWidget *parent = nullptr);
public slots:
    void slot_OpenFile(const QString& path);
private:
    std::shared_ptr<FileListThread> _thread_openfiles;
};

#endif // FILELISTWIDGET_H
