#ifndef FILETREEITEM_H
#define FILETREEITEM_H
#include <QTreeWidgetItem>
class FileTreeItem : public QTreeWidgetItem
{
public:
    /*
    explicit QListWidgetItem(QListWidget *listview = nullptr, int type = Type);
    explicit QListWidgetItem(const QString &text, QListWidget *listview = nullptr, int type = Type);
    explicit QListWidgetItem(const QIcon &icon, const QString &text,
                             QListWidget *listview = nullptr, int type = Type);*/
    explicit FileTreeItem(QTreeWidget *listview = nullptr, int type = Type);
    explicit FileTreeItem(QTreeWidgetItem *parent, int type);
};

#endif // FILETREEITEM_H
