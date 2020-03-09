#ifndef GWMPROPERTYPANEL_H
#define GWMPROPERTYPANEL_H

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#include <QTabWidget>
#include <QStandardItemModel>
#include <qgsvectorlayer.h>
#include <PropertyPanelTabs/gwmpropertydefaulttab.h>

namespace Ui {
class GwmPropertyPanel;
}

class GwmPropertyPanel : public QTabWidget
{
    Q_OBJECT

public:
    explicit GwmPropertyPanel(QWidget *parent = nullptr, QStandardItemModel* model = new QStandardItemModel);
    ~GwmPropertyPanel();

private:
    GwmPropertyDefaultTab* defaultTab;
    QStandardItemModel* mapModel;

public:
    void addStatisticTab(QModelIndex index, QgsVectorLayer* layer);

private:
    bool isDefaultTabShow;
    void manageDefaultTab();

};

#endif // GWMPROPERTYPANEL_H
