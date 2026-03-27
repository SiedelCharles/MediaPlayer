#ifndef FILELISTITEM_H
#define FILELISTITEM_H
#include <QListWidgetItem>
class FileListItem : public QListWidgetItem
{
public:
    /*
    explicit QListWidgetItem(QListWidget *listview = nullptr, int type = Type);
    explicit QListWidgetItem(const QString &text, QListWidget *listview = nullptr, int type = Type);
    explicit QListWidgetItem(const QIcon &icon, const QString &text,
                             QListWidget *listview = nullptr, int type = Type);*/
    FileListItem(QListWidget *listview = nullptr, int type = Type);
};

#endif // FILELISTITEM_H
