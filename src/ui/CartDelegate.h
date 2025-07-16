#ifndef CARTDELEGATE_H
#define CARTDELEGATE_H

#include <QStyledItemDelegate>

class CartDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit CartDelegate(QObject *parent = nullptr);

    // Override to create our own editors
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    // Override to paint our custom "Remove" button
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    // Override to handle mouse events for our button
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

signals:
    void removeItem(const QModelIndex &index);

};

#endif // CARTDELEGATE_H 