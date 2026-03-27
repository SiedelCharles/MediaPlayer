#ifndef SETTING_H
#define SETTING_H

#endif // SETTING_H

#include <QListWidgetItem>

enum class ItemListType {
    ItemListTypeSide = QListWidgetItem::UserType+1,
    ItemListTypeFileFolder = QListWidgetItem::UserType+2,
    ItemListTypeFileAudio = QListWidgetItem::UserType+3,
};

const int SIDE_LENGTH = 50;
