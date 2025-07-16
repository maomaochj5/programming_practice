#include "CartDelegate.h"
#include <QSpinBox>
#include <QApplication>
#include <QMouseEvent>
#include <QPainter>

CartDelegate::CartDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *CartDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == 1) { // Quantity column
        QSpinBox *editor = new QSpinBox(parent);
        editor->setFrame(false);
        editor->setMinimum(1);
        editor->setMaximum(999);
        return editor;
    }
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void CartDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (index.column() == 1) {
        int value = index.model()->data(index, Qt::EditRole).toInt();
        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        spinBox->setValue(value);
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

void CartDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (index.column() == 1) {
        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        spinBox->interpretText();
        int value = spinBox->value();
        model->setData(index, value, Qt::EditRole);
    } else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}

void CartDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

void CartDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == 4) { // Actions column
        QStyleOptionButton button;
        button.rect = option.rect.adjusted(4, 4, -4, -4);
        button.text = "移除";
        button.state = QStyle::State_Enabled;

        QApplication::style()->drawControl(QStyle::CE_PushButton, &button, painter);
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

bool CartDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (index.column() == 4 && event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (option.rect.contains(mouseEvent->pos())) {
            emit removeItem(index);
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
} 