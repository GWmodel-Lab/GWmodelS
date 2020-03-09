#ifndef GWMPROPERTYPANEL_H
#define GWMPROPERTYPANEL_H

#include <QTabWidget>
#include <QStandardItemModel>
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
    void addStatisticTab(QModelIndex index);

private:
    void setupTabs();

};

#endif // GWMPROPERTYPANEL_H
