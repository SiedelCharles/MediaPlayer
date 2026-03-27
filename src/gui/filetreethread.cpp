#include "filetreeitem.h"
#include "filetreethread.h"
#include "filetreewidget.h"
#include <QDir>
#include <QDebug>
#include "setting.h"

FileTreeThread::FileTreeThread( const QString &src_path, QObject *parent, QTreeWidget *list)
    :_src_path(src_path), QThread(parent), _list(list)
{
    // connect(this, &FileTreeThread::SigOpenFileFinished, this, &QThread::deleteLater);
}

FileTreeThread::~FileTreeThread()
{
    // qDebug() << "Thread is destructed.";
}

void FileTreeThread::run()
{
    QDir src_dir(_src_path);
    auto name = src_dir.dirName();
    _root = new FileTreeItem(dynamic_cast<QTreeWidget*>(_list), static_cast<int>(ItemListType::ItemListTypeFileFolder));
    _root->setData(0, Qt::DisplayRole, name);
    _root->setData(0, Qt::DecorationRole, QIcon(":/resource/133397324_p0_master1200.jpg"));
    _root->setData(0, Qt::ToolTipRole, _src_path);
    OpenFiles(_src_path);
    emit SigOpenFileFinished();
}

void FileTreeThread::OpenFiles(const QString &path)
{

    QDir src_dir(path);
    src_dir.setFilter(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot);
    src_dir.setSorting(QDir::Name);
    QFileInfoList info_list = src_dir.entryInfoList();
    for(const auto& iter : qAsConst(info_list)) {
        if(iter.isDir()) {
            OpenFiles(iter.filePath());
        } else {
            const QString& suffix = iter.completeSuffix();
            if(suffix!="mp4" && suffix!="mp3" && suffix!="wav") {
                continue;
            }
            auto *item = new FileTreeItem(_root, static_cast<int>(ItemListType::ItemListTypeFileAudio));
            item->setData(0, Qt::DisplayRole, iter.fileName());
            item->setData(0, Qt::DecorationRole, QIcon(":/resource/98622613_p0_master1200.jpg"));
            item->setData(0, Qt::ToolTipRole, iter.absoluteFilePath());
        }
    }
}
