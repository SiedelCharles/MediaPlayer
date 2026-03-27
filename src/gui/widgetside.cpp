#include "setting.h"
#include "widgetside.h"
#include "delegateside.h"
#include "itemsidelistwidget.h"
#include <QDebug>
#include <QPixmap>

WidgetSide::WidgetSide(QWidget *parent)
    : QListWidget(parent)
{
    // 取消虚线框
    setFocusPolicy(Qt::NoFocus);
    setIconSize(QSize(SIDE_LENGTH, SIDE_LENGTH));

    auto *delegate = new DelegateSide(QSize(SIDE_LENGTH, SIDE_LENGTH), 10, this);
    this->setItemDelegate(delegate);

    _item_display = new ItemSideListWidget(this, static_cast<int>(ItemListType::ItemListTypeSide));
    _item_transcription = new ItemSideListWidget(this, static_cast<int>(ItemListType::ItemListTypeSide));
    _item_translation = new ItemSideListWidget(this, static_cast<int>(ItemListType::ItemListTypeSide));
    _item_synthesis = new ItemSideListWidget(this, static_cast<int>(ItemListType::ItemListTypeSide));

    QPixmap pix;
    QIcon icon;
    pix = QPixmap(":/resource/88483602_p0_master1200.jpg");
    icon = QIcon(pix);
    icon.addPixmap(pix, QIcon::Selected);
    _item_display->setIcon(icon);
    pix = QPixmap(":/resource/101904007_p0.jpg");
    icon = QIcon(pix);
    icon.addPixmap(pix, QIcon::Selected);
    _item_transcription->setIcon(icon);
    pix = QPixmap(":/resource/115921744_p0_master1200.jpg");
    icon = QIcon(pix);
    icon.addPixmap(pix, QIcon::Selected);
    _item_translation->setIcon(icon);
    pix = QPixmap(":/resource/processed1.jpg");
    icon = QIcon(pix);
    icon.addPixmap(pix, QIcon::Selected);
    _item_synthesis->setIcon(icon);

    this->addItem(_item_display);
    this->addItem(_item_transcription);
    this->addItem(_item_translation);
    this->addItem(_item_synthesis);
}
