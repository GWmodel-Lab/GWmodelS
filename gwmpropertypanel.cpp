#include "gwmpropertypanel.h"
#include "PropertyPanelTabs/gwmpropertystatisticstab.h"

GwmPropertyPanel::GwmPropertyPanel(QWidget *parent) :
    QTabWidget(parent),
    mDefaultTab(new GwmPropertyDefaultTab),
    mMapModel(new GwmLayerItemModel),
    isDefaultTabShow(false)
{
    manageDefaultTab();
    connect(this, &QTabWidget::tabCloseRequested, this, &GwmPropertyPanel::onTabCloseRequest);
}

GwmPropertyPanel::~GwmPropertyPanel()
{
}

GwmLayerItemModel *GwmPropertyPanel::mapModel() const
{
    return mMapModel;
}

void GwmPropertyPanel::setMapModel(GwmLayerItemModel *mapModel)
{
    if (!isMapModelSetted)
        delete mMapModel;
    mMapModel = mapModel;
    isMapModelSetted = true;
}

void GwmPropertyPanel::manageDefaultTab()
{
    auto defaultTabIndex = indexOf(mDefaultTab);
    if (isDefaultTabShow)
    {
        removeTab(defaultTabIndex);
        isDefaultTabShow = false;
        setTabsClosable(true);
    }
    else if (count() < 1)
    {
        addTab(mDefaultTab, "Default");
        isDefaultTabShow = true;
        setTabsClosable(false);
    }
}

void GwmPropertyPanel::onTabCloseRequest(int index)
{
    if (index != indexOf(mDefaultTab))
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
    GwmLayerItem* item = mMapModel->itemFromIndex(index);
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
