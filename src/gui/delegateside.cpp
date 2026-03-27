#include "delegateside.h"

DelegateSide::DelegateSide(QSize icon_size, int vertical_padding, QObject *parent)
    : QStyledItemDelegate(parent), _size_icon(icon_size), _padding_vertical(vertical_padding) {}

QSize DelegateSide::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(0, _size_icon.height() + 2 * _padding_vertical);
}

void DelegateSide::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    if (icon.isNull()) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, option.widget);

    QRect rect_iocn = option.rect;
    rect_iocn.setSize(_size_icon);
    rect_iocn.moveCenter(option.rect.center());

    icon.paint(painter, rect_iocn, Qt::AlignCenter, QIcon::Normal, QIcon::On);
}

