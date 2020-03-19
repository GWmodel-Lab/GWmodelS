#ifndef GWMLAYERSYMBOLITEM_H
#define GWMLAYERSYMBOLITEM_H

#include <QObject>
#include <qgssymbol.h>
#include "gwmlayeritem.h"

class GwmLayerVectorItem;

class GwmLayerSymbolItem : public GwmLayerItem
{
    Q_OBJECT
public:
    GwmLayerSymbolItem(GwmLayerVectorItem* parentItem = nullptr, QIcon symbol = QIcon(), QString label = QStringLiteral(""));

public:
    virtual QString text();
    virtual QVariant data(int col, int role);
    inline virtual Qt::ItemFlags flags() override;

    virtual GwmLayerItem * child(int row) override;
    virtual int childCount();
    virtual int childNumber();

    virtual bool insertChildren(int position, int count) override;
    virtual bool removeChildren(int position, int count) override;

    virtual bool insertChildren(int position, QList<GwmLayerItem*> items) override;
    virtual bool appendChildren(QList<GwmLayerItem*> items) override;
    virtual QList<GwmLayerItem*> takeChildren(int position, int count) override;


    inline QIcon symbol() const;
    inline void setSymbol(QIcon symbol);

    inline QString label() const;
    inline void setLabel(const QString &label);

    inline virtual GwmLayerItemType itemType() { return GwmLayerItemType::Symbol; }

private:
    QIcon mSymbol;
    QString mLabel;
};

inline Qt::ItemFlags GwmLayerSymbolItem::flags()
{
    return Qt::ItemNeverHasChildren | GwmLayerItem::flags();
}

inline QIcon GwmLayerSymbolItem::symbol() const
{
    return mSymbol;
}

inline void GwmLayerSymbolItem::setSymbol(QIcon symbol)
{
    mSymbol = symbol;
}

inline QString GwmLayerSymbolItem::label() const
{
    return mLabel;
}

inline void GwmLayerSymbolItem::setLabel(const QString &label)
{
    mLabel = label;
}

#endif // GWMLAYERSYMBOLITEM_H
