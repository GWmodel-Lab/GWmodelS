#ifndef GWMLAYERATTRIBUTEITEM_H
#define GWMLAYERATTRIBUTEITEM_H

#include <QObject>
#include <QVariant>
#include <qgsattributetableconfig.h>

class GwmLayerAttributeItem : public QObject
{
    Q_OBJECT
public:
    explicit GwmLayerAttributeItem(int index = 0, const QString attributeName = "", const QgsAttributeTableConfig attributeConfig = *(new QgsAttributeTableConfig()),QObject *parent = nullptr);

public:
    virtual QString text();
    virtual QVariant data(int col, int role);
    virtual Qt::ItemFlags flags();
    virtual bool setData(int col, int role, QVariant value);
    int index();
    GwmLayerAttributeItem* clone();
    QgsAttributeTableConfig::Type type();

    int attributeIndex() const;
    void setAttributeIndex(int attributeIndex);

    QString attributeName() const;
    void setAttributeName(const QString &attributeName);

    QgsAttributeTableConfig::Type columnType() const;
    void setColumnType(const QgsAttributeTableConfig::Type &columnType);

    QgsAttributeTableConfig attributeTableConfig() const;

signals:

private:
    int mAttributeIndex;
    QString mAttributeName;
    QgsAttributeTableConfig::Type mColumnType;
    const QgsAttributeTableConfig mAttributeTableConfig;
};

#endif // GWMLAYERATTRIBUTEITEM_H
