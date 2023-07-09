#include "gwmlayoutbatchconfigurationdelegate.h"

#include <QApplication>
#include <QPushButton>
#include <QPainter>
#include <QLineEdit>

#include <qgsrenderer.h>
#include <qgsgraduatedsymbolrenderer.h>

QMap<QString, GwmLayoutBatchConfigurationDelegate::PaintSymbolColumnFunction> GwmLayoutBatchConfigurationDelegate::PaintSymbolColumnFunctionMap =
{
    std::make_pair("singleSymbol", &GwmLayoutBatchConfigurationDelegate::paintSingleSymbolColumn),
    std::make_pair("graduatedSymbol", &GwmLayoutBatchConfigurationDelegate::paintGraduatedSymbolColumn)
};

GwmLayoutBatchConfigurationItem::Type GwmLayoutBatchConfigurationDelegate::typeFromIndex(const QModelIndex &index)
{
    auto item = (GwmLayoutBatchConfigurationItem*)index.internalPointer();
    return item->type();
}

GwmLayoutBatchConfigurationDelegate::GwmLayoutBatchConfigurationDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    mIconSize = QSize(12, 12);
}

void GwmLayoutBatchConfigurationDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (typeFromIndex(index) == GwmLayoutBatchConfigurationItem::Type::Field)
    {
        if (index.column() == 1)
        {
            QgsFeatureRenderer* renderer = (QgsFeatureRenderer*)index.data().value<void*>();
            auto paintSymbol = PaintSymbolColumnFunctionMap.value(renderer->type(), &GwmLayoutBatchConfigurationDelegate::paintDefaultSymbolColumn);
            (this->*paintSymbol)(renderer, painter, option);
        }
        else QStyledItemDelegate::paint(painter, option, index);
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize GwmLayoutBatchConfigurationDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (typeFromIndex(index) == GwmLayoutBatchConfigurationItem::Type::Field)
    {
        if (index.column() == 1)
            return QSize(20, 40);
    }
    return QStyledItemDelegate::sizeHint(option, index);
}

//QWidget *GwmLayoutBatchConfigurationDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
//{
//    if (typeFromIndex(index) == GwmLayoutBatchConfigurationItem::Type::Field)
//    {
//        if (index.column() == 1)
//        {
//            QLineEdit* lineEditor = new QLineEdit();
//            return lineEditor;
//        }
//    }
//    else return QStyledItemDelegate::createEditor(parent, option, index);
//}

//void GwmLayoutBatchConfigurationDelegate::destroyEditor(QWidget *editor, const QModelIndex &index) const
//{
//    if (typeFromIndex(index) == GwmLayoutBatchConfigurationItem::Type::Field)
//    {
//        if (index.column() == 1)
//        {
//            QLineEdit* lineEditor = static_cast<QLineEdit*>(editor);
//            delete lineEditor;
//        }
//    }
//    else QStyledItemDelegate::destroyEditor(editor, index);
//}

//void GwmLayoutBatchConfigurationDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
//{
//    if (typeFromIndex(index) == GwmLayoutBatchConfigurationItem::Type::Field)
//    {
//        if (index.column() == 1)
//        {
//            QgsFeatureRenderer* renderer = (QgsFeatureRenderer*)index.data().value<void*>();
//            QLineEdit* lineEditor = static_cast<QLineEdit*>(editor);
//            lineEditor->setText(renderer->type());
//        }
//    }
//    else QStyledItemDelegate::setEditorData(editor, index);
//}

//void GwmLayoutBatchConfigurationDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
//{
//}

void GwmLayoutBatchConfigurationDelegate::paintDefaultSymbolColumn(QgsFeatureRenderer *renderer, QPainter *painter, const QStyleOptionViewItem &option) const
{
    QApplication::style()->drawItemText(painter, option.rect, Qt::AlignLeft | Qt::AlignVCenter, Qt::GlobalColor::black, true, renderer->type());
}

void GwmLayoutBatchConfigurationDelegate::paintSingleSymbolColumn(QgsFeatureRenderer *renderer, QPainter *painter, const QStyleOptionViewItem &option) const
{
    auto symbolItem = renderer->legendSymbolItems().first();
    QPixmap pixmap(mIconSize);
    pixmap.fill(Qt::GlobalColor::transparent);
    QPainter iconPainter(&pixmap);
    symbolItem.symbol()->drawPreviewIcon(&iconPainter, mIconSize);
    QApplication::style()->drawItemPixmap(painter, option.rect, Qt::AlignCenter | Qt::AlignVCenter, pixmap);
}

void GwmLayoutBatchConfigurationDelegate::paintGraduatedSymbolColumn(QgsFeatureRenderer *renderer, QPainter *painter, const QStyleOptionViewItem &option) const
{
    QgsGraduatedSymbolRenderer* graduatedRenderer = static_cast<QgsGraduatedSymbolRenderer*>(renderer);
    QList<QgsLegendSymbolItem> symbolItemList = graduatedRenderer->legendSymbolItems();
    QgsRangeList symbolRangeList = graduatedRenderer->ranges();
    QRect fullZone = option.rect;
    int symbolCount = symbolItemList.size(), cellWidth = fullZone.width() / ((symbolCount + 1) * 2);
    for (int i = 0; i < symbolCount; i++)
    {
        // 绘制图标
        int iconLeft = fullZone.left() + i * cellWidth * 2 + cellWidth, iconTop = fullZone.top();
        QRect iconZone(iconLeft, iconTop, cellWidth * 2, fullZone.height() / 2);
        auto symbolItem = symbolItemList[i];
        QPixmap pixmap(mIconSize);
        pixmap.fill(Qt::GlobalColor::transparent);
        QPainter iconPainter(&pixmap);
        symbolItem.symbol()->drawPreviewIcon(&iconPainter, mIconSize);
        QApplication::style()->drawItemPixmap(painter, iconZone, Qt::AlignCenter | Qt::AlignVCenter, pixmap);
        // 绘制文本
        int textLeft = fullZone.left() + i * cellWidth * 2, textTop = fullZone.top() + fullZone.height() / 2;
        QRect textZone(textLeft, textTop, cellWidth * 2, fullZone.height() / 2);
        QString lowerLabel = QString("%1").arg(symbolRangeList[i].lowerValue(), 0, 'f', 1);
        QApplication::style()->drawItemText(painter, textZone, Qt::AlignCenter | Qt::AlignVCenter, Qt::GlobalColor::black, true, lowerLabel);
    }
    int textLeft = fullZone.left() + symbolCount * cellWidth * 2;
    QRect textZone(textLeft, fullZone.top() + fullZone.height() / 2, cellWidth * 2, fullZone.height() / 2);
    QString lowerLabel = QString("%1").arg(symbolRangeList[symbolCount - 1].upperValue(), 0, 'f', 1);
    QApplication::style()->drawItemText(painter, textZone, Qt::AlignCenter | Qt::AlignVCenter, Qt::GlobalColor::black, true, lowerLabel);
}

//void GwmLayoutBatchCOnfigurationDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
//{
//}
