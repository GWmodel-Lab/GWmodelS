#ifndef GWMLAYOUTBATCHCONFIGURATIONDELEGATE_H
#define GWMLAYOUTBATCHCONFIGURATIONDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>
#include "Layout/gwmlayoutbatchconfigurationmodel.h"

class GwmLayoutBatchConfigurationDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    typedef void (GwmLayoutBatchConfigurationDelegate::*PaintSymbolColumnFunction)(QgsFeatureRenderer*, QPainter*, const QStyleOptionViewItem&) const;
    static QMap<QString, PaintSymbolColumnFunction> PaintSymbolColumnFunctionMap;

private:
    static GwmLayoutBatchConfigurationItem::Type typeFromIndex(const QModelIndex& index);

public:
    GwmLayoutBatchConfigurationDelegate(QObject * parent = nullptr);

    // QAbstractItemDelegate interface
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
//    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
//    void destroyEditor(QWidget *editor, const QModelIndex &index) const override;
//    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
//    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
//    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void paintDefaultSymbolColumn(QgsFeatureRenderer* renderer, QPainter* painter, const QStyleOptionViewItem& option) const;
    void paintSingleSymbolColumn(QgsFeatureRenderer* renderer, QPainter* painter, const QStyleOptionViewItem& option) const;
    void paintGraduatedSymbolColumn(QgsFeatureRenderer* renderer, QPainter* painter, const QStyleOptionViewItem& option) const;

private:
    QSize mIconSize;
};

#endif // GWMLAYOUTBATCHCONFIGURATIONDELEGATE_H
