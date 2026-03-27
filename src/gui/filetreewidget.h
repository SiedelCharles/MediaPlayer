#ifndef FILETREEWIDGET_H
#define FILETREEWIDGET_H

#include <QHeaderView>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include "filetreethread.h"

class FileTreeWidget : public QTreeWidget
{
    Q_OBJECT
signals:
    // void SigPlayMusic(QTreeWidgetItem *item, int cloumn);
public:
    FileTreeWidget(QWidget *parent = nullptr);
    void Init();
public slots:
    void slot_OpenFile(const QString& path);
    void slot_AddFile(const QString& path);
private:
    std::shared_ptr<FileTreeThread> _thread_openfiles;
    QTreeWidgetItem *_root = nullptr;
};


#endif // FILETREEWIDGET_H
