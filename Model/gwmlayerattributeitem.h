#ifndef GWMLAYERATTRIBUTEITEM_H
#define GWMLAYERATTRIBUTEITEM_H

#include <QObject>
#include <QVariant>

class GwmLayerAttributeItem : public QObject
{
    Q_OBJECT
public:
    explicit GwmLayerAttributeItem(int index = 0, const QString attributeName = "", const QString type = "",QObject *parent = nullptr);

public:
    virtual QString text();
    virtual QVariant data(int col, int role);
    virtual Qt::ItemFlags flags();
    virtual bool setData(int col, int role, QVariant value);
    int index();
    GwmLayerAttributeItem* clone();
signals:

private:
    int attributeIndex;
    QString attributeName;
    QString dataType;
};

#endif // GWMLAYERATTRIBUTEITEM_H
