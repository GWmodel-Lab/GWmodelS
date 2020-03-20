#ifndef GWMPROPERTYPANEL_H
#define GWMPROPERTYPANEL_H

#include "prefix.h"

#include <QTabWidget>
#include <QStandardItemModel>
#include <qgsvectorlayer.h>
#include <PropertyPanelTabs/gwmpropertydefaulttab.h>
#include "Model/gwmlayeritemmodel.h"

namespace Ui {
class GwmPropertyPanel;
}

class GwmPropertyPanel : public QTabWidget
{
    Q_OBJECT

public:
    explicit GwmPropertyPanel(QWidget *parent = nullptr);
    ~GwmPropertyPanel();

private:
    GwmPropertyDefaultTab* mDefaultTab;
    GwmLayerItemModel* mMapModel;
    bool isMapModelSetted;

public:
    void addPropertyTab(const QModelIndex& index);

    GwmLayerItemModel *mapModel() const;
    void setMapModel(GwmLayerItemModel *mapModel);

private:
    bool isDefaultTabShow;
    void manageDefaultTab();

private slots:
    void onTabCloseRequest(int index);

};

#endif // GWMPROPERTYPANEL_H
