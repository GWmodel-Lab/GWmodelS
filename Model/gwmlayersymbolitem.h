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
    GwmLayerSymbolItem(GwmLayerVectorItem* parentItem = nullptr, QgsSymbol* symbol = nullptr, QString label = QStringLiteral(""));

public:
    virtual QString text();
    virtual QVariant data(int col, int role);
    virtual GwmLayerItem * child(int row) override;
    virtual int childCount();
    virtual int childNumber();

    virtual bool insertChildren(int position, int count) override;
    virtual bool removeChildren(int position, int count) override;


    inline QgsSymbol *symbol() const;
    inline void setSymbol(QgsSymbol *symbol);

    inline QString label() const;
    inline void setLabel(const QString &label);

private:
    QgsSymbol* mSymbol;
    QString mLabel;
};

#endif // GWMLAYERSYMBOLITEM_H
