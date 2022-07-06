#ifndef GWMLAYERATTRIBUTEITEM_H
#define GWMLAYERATTRIBUTEITEM_H

#include <QObject>
#include <QVariant>
//#include <qgsattributetableconfig.h>

class GwmLayerAttributeItem : public QObject
{
    Q_OBJECT
public:
    explicit GwmLayerAttributeItem(int index = 0, const QString attributeName = "",const QVariant::Type type = QVariant::Type::String,QObject *parent = nullptr);

public:
    virtual QString text();
    virtual QVariant data(int col, int role);
    virtual Qt::ItemFlags flags();
    virtual bool setData(int col, int role, QVariant value);
    int index();
    GwmLayerAttributeItem* clone();

    int attributeIndex() const;
    void setAttributeIndex(int attributeIndex);

    QString attributeName() const;
    void setAttributeName(const QString &attributeName);

    QVariant::Type attributeType() const;
    void setAttributeType(const QVariant::Type &columnType);

    QVariant::Type type();
signals:

private:
    int mAttributeIndex;
    QString mAttributeName;
    QVariant::Type mAttributeType;
};

#endif // GWMLAYERATTRIBUTEITEM_H
