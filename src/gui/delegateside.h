#ifndef DELEGATESIDE_H
#define DELEGATESIDE_H
#include <QDebug>
#include <QPainter>
#include <QListWidget>
#include <QApplication>
#include <QStyledItemDelegate>

class DelegateSide : public QStyledItemDelegate
{
public:
    DelegateSide(QSize icon_size, int vertical_padding, QObject *parent = nullptr);
protected:
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
private:
    QSize _size_icon;
    int _padding_vertical;
};

#endif // DELEGATESIDE_H
