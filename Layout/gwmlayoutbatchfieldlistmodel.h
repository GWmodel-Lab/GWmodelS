#ifndef GWMLAYOUTBATCHFIELDLISTMODEL_H
#define GWMLAYOUTBATCHFIELDLISTMODEL_H

#include <QAbstractListModel>

#include <qgsfield.h>
#include <qgsvectorlayer.h>

class QgsFieldCheckable : public QgsField
{
public:
    QgsFieldCheckable(const QgsField& field);

    bool checked() const
    {
        return mChecked;
    }
    void setChecked(bool checked)
    {
        mChecked = checked;
    }

private:
    bool mChecked = false;
};

class GwmLayoutBatchFieldListModel : public QAbstractListModel
{
    Q_OBJECT

public:

public:
    explicit GwmLayoutBatchFieldListModel(QgsVectorLayer* layer, QObject *parent = nullptr);
    ~GwmLayoutBatchFieldListModel();

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QgsFieldCheckable* itemFromIndex(const QModelIndex& index) const;
    int checkedIndex(const QgsFieldCheckable* field);

private:
    QList<QgsFieldCheckable*> mFieldList;
};

#endif // GWMLAYOUTBATCHFIELDLISTMODEL_H
