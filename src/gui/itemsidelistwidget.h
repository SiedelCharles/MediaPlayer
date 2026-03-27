#ifndef ITEMSIDELISTWIDGET_H
#define ITEMSIDELISTWIDGET_H

#include <QListWidgetItem>

class ItemSideListWidget : public QListWidgetItem
{
public:
    ItemSideListWidget(QListWidget *listview = nullptr, int type = Type);
};

#endif // ITEMSIDELISTWIDGET_H
