#include "filetreeitem.h"
#include "filetreewidget.h"

FileTreeItem::FileTreeItem(QTreeWidget *listview, int type)
: QTreeWidgetItem(listview, type)
{

}

FileTreeItem::FileTreeItem(QTreeWidgetItem *parent, int type)
: QTreeWidgetItem(parent, type)
{

}
