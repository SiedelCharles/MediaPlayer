#include "filetreewidget.h"
#include "filetreethread.h"

#include <QDir>
#include "setting.h"
#include "filetreeitem.h"

FileTreeWidget::FileTreeWidget(QWidget *parent)
    : QTreeWidget(parent), _thread_openfiles(nullptr)
{
}

void FileTreeWidget::Init()
{
    _root = new FileTreeItem(this, static_cast<int>(ItemListType::ItemListTypeFileFolder));
    _root->setData(0, Qt::DisplayRole, "Files");
    _root->setData(0, Qt::DecorationRole, QIcon(":/resource/133397324_p0_master1200.jpg"));
    _root->setData(0, Qt::ToolTipRole, "Opened Files");
}

void FileTreeWidget::slot_OpenFile(const QString &path)
{
    // TODO:to create a set to store the opened files;
    QDir project_dir(path);
    // QString project_name = project_dir.dirName();
    _thread_openfiles = std::make_shared<FileTreeThread>(path, nullptr, this);
    connect(_thread_openfiles.get(), &FileTreeThread::SigOpenFileFinished, this, [this]() {
        _thread_openfiles.reset();
    });
    if(_thread_openfiles!=nullptr) {
       _thread_openfiles->start();
    }
}

void FileTreeWidget::slot_AddFile(const QString &path)
{
    QFile new_file(path);
    if(new_file.exists()) {
        QString file_name = new_file.fileName();
        auto *item = new FileTreeItem(_root, static_cast<int>(ItemListType::ItemListTypeFileAudio));
        item->setData(0, Qt::DisplayRole, file_name);
        item->setData(0, Qt::DecorationRole, QIcon(":/resource/98622613_p0_master1200.jpg"));
        item->setData(0, Qt::ToolTipRole, path);
        qDebug() << "Item's parent:" << item->parent();
    }

}

