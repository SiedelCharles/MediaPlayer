#include "filelistwidget.h"
#include "filelistthread.h"

#include <QDir>

FileListWidget::FileListWidget(QWidget *parent)
    : QListWidget(parent), _thread_openfiles(nullptr)
{}

void FileListWidget::slot_OpenFile(const QString &path)
{
    // TODO:to create a set to store the opened files;
    QDir project_dir(path);
    QString project_name = project_dir.dirName();
    _thread_openfiles = std::make_shared<FileListThread>(path, nullptr, this);
    _thread_openfiles->start();
}
