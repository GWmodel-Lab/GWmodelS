#include "gwmpropertypanel.h"
#include "PropertyPanelTabs/gwmpropertystatisticstab.h"

GwmPropertyPanel::GwmPropertyPanel(QWidget *parent, GwmLayerItemModel* model) :
    QTabWidget(parent),
    defaultTab(new GwmPropertyDefaultTab),
    mapModel(model),
    isDefaultTabShow(false)
{
    manageDefaultTab();
    connect(this, &QTabWidget::tabCloseRequested, this, &GwmPropertyPanel::onTabCloseRequest);
}

GwmPropertyPanel::~GwmPropertyPanel()
{
}

void GwmPropertyPanel::manageDefaultTab()
{
    auto defaultTabIndex = indexOf(defaultTab);
    if (isDefaultTabShow)
    {
        removeTab(defaultTabIndex);
        isDefaultTabShow = false;
        setTabsClosable(true);
    }
    else if (count() < 1)
    {
        addTab(defaultTab, "Default");
        isDefaultTabShow = true;
        setTabsClosable(false);
    }
}

void GwmPropertyPanel::onTabCloseRequest(int index)
{
    if (index != indexOf(defaultTab))
    {
        removeTab(index);
        manageDefaultTab();
    }
}

void GwmPropertyPanel::addPropertyTab(const QModelIndex& index)
{
    qDebug() << "[GwmPropertyPanel::addPropertyTab]"
             << "index:" << index;
    // 创建标签页
    GwmLayerItem* item = mapModel->itemFromIndex(index);
    QString tabName = item->text();
    QWidget* tabWidget = nullptr;
    switch (item->itemType())
    {
    case GwmLayerItem::Origin:
        tabWidget = new GwmPropertyStatisticsTab(this, (GwmLayerOriginItem*)item);
        tabName = ((GwmLayerOriginItem*)item)->parentItem()->text();
        break;
    case GwmLayerItem::Group:
        tabWidget = new GwmPropertyStatisticsTab(this, ((GwmLayerGroupItem*)item)->originChild());
    default:
        break;
    }
    addTab(tabWidget, tabName);

    // 默认页
    manageDefaultTab();
}
