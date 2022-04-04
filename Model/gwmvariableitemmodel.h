#ifndef GWMVARIABLEITEMMODEL_H
#define GWMVARIABLEITEMMODEL_H

#include <QAbstractListModel>
#include <qgsvectorlayer.h>

struct GwmVariable
{
    int index;
    QString name;
    QVariant::Type type;
    bool isNumeric;
};

class GwmVariableItemModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit GwmVariableItemModel(QObject *parent = nullptr);
    explicit GwmVariableItemModel(QgsVectorLayer* layer, QObject *parent = nullptr);
    explicit GwmVariableItemModel(GwmVariableItemModel* indepVarModelX, GwmVariableItemModel* indepVarModelY, QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool insert(int row, GwmVariable variable);
    bool insert(int row, QList<GwmVariable> variables);

    bool append(GwmVariable variable);
    bool append(QList<GwmVariable> variables);

    bool remove(int row);
    bool remove(int row, int count);

    GwmVariable itemFromIndex(const QModelIndex& index) const;
    QList<GwmVariable> attributeItemList() const;

    GwmVariable item(int i) const;

    bool clear();

protected:

    // Add data:
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

private:
    QStringList mHorizontalHeaderData = { "" };
    QList<GwmVariable> mItems;
};

#endif // GWMVARIABLEITEMMODEL_H
