#include "filelistitem.h"
#include "filelistthread.h"
#include "filelistwidget.h"
#include <QDir>
#include <QDebug>
#include "setting.h"

FileListThread::FileListThread( const QString &src_path, QObject *parent, QListWidget *list)
    :_src_path(src_path), QThread(parent), _list(list)
{
    QDir src_dir(_src_path);
    auto name = src_dir.dirName();
    auto *item = new FileListItem(list, static_cast<int>(ItemListType::ItemListTypeFileFolder));
    item->setData(Qt::DisplayRole, name);
    item->setData(Qt::DecorationRole, QIcon(":/resource/133397324_p0_master1200.jpg"));
    item->setData(Qt::ToolTipRole, _src_path);
    OpenFiles(src_path);
}

void FileListThread::OpenFiles(const QString &path)
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
            auto *item = new FileListItem(dynamic_cast<FileListWidget*>(_list), static_cast<int>(ItemListType::ItemListTypeFileAudio));
            item->setData(Qt::DisplayRole, iter.fileName());
            item->setData(Qt::DecorationRole, QIcon(":/resource/98622613_p0_master1200.jpg"));
            item->setData(Qt::ToolTipRole, iter.absoluteFilePath());
        }
    }
}
