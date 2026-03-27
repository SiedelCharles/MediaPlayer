#include "itemsidelistwidget.h"
#include "setting.h"

ItemSideListWidget::ItemSideListWidget(QListWidget *listview, int type)
    : QListWidgetItem(listview, type)
{
    this->setSizeHint(QSize(SIDE_LENGTH, SIDE_LENGTH));
}
