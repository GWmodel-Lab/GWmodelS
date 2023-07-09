#include "gwmpropertypanel.h"
#include "PropertyPanelTabs/gwmpropertystatisticstab.h"
#include "PropertyPanelTabs/gwmpropertygwrtab.h"
#include "PropertyPanelTabs/gwmpropertyscalablegwrtab.h"
#include "PropertyPanelTabs/gwmpropertyggwrtab.h"
#include "PropertyPanelTabs/gwmpropertygtwrtab.h"
#include "PropertyPanelTabs/gwmpropertymultiscalegwrtab.h"
#include "PropertyPanelTabs/gwmpropertygwsstab.h"
#include "PropertyPanelTabs/gwmpropertycollinearitygwrtab.h"
#include "PropertyPanelTabs/gwmpropertygwcorrelationstab.h"
#include "PropertyPanelTabs/gwmpropertygwpcatab.h"

GwmPropertyPanel::GwmPropertyPanel(QWidget *parent) :
    QTabWidget(parent),
    mDefaultTab(new GwmPropertyDefaultTab),
    mMapModel(nullptr)
{
    manageDefaultTab();
    connect(this, &QTabWidget::tabCloseRequested, this, &GwmPropertyPanel::onTabCloseRequest);
    connect(this, &QTabWidget::currentChanged,this,&QTabWidget::setCurrentIndex);
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
    mMapModel = mapModel;
    connect(mMapModel,&GwmLayerItemModel::layerpropertyRemovedSignal,this, &GwmPropertyPanel::onLayerTabCloseRequest);
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

void GwmPropertyPanel::onLayerTabCloseRequest(QWidget* tabWidget)
{
    if (tabWidget != nullptr )
    {
        int index = indexOf(tabWidget);
        removeTab(index);
        manageDefaultTab();
    }
}

void GwmPropertyPanel::addPropertyTab(const QModelIndex& index)
{
    qDebug() << "[GwmPropertyPanel::addPropertyTab]"
             << "index:" << index;
    // 创建标签页
    if (mMapModel)
    {
        GwmLayerItem* item = mMapModel->itemFromIndex(index);
        if (item)
        {
            QString tabName = item->text();
            QWidget* tabWidget = nullptr;
            tabWidget = item->tabWidget;
            if(tabWidget == nullptr){
                switch (item->itemType())
                {
                case GwmLayerItem::Origin:
                    tabWidget = new GwmPropertyStatisticsTab(this, static_cast<GwmLayerOriginItem*>(item));
                    tabName = ((GwmLayerOriginItem*)item)->parentItem()->text();
                    break;
                case GwmLayerItem::Group:
                    tabWidget = new GwmPropertyStatisticsTab(this, (static_cast<GwmLayerGroupItem*>(item))->originChild());
                    break;
                case GwmLayerItem::GWR:
                    tabWidget = new GwmPropertyGWRTab(this, static_cast<GwmLayerBasicGWRItem*>(item));
                    (static_cast<GwmPropertyGWRTab*>(tabWidget))->updateUI();
                    break;
                case GwmLayerItem::ScalableGWR:
                    tabWidget = new GwmPropertyScalableGWRTab(this, static_cast<GwmLayerScalableGWRItem*>(item));
                    (static_cast<GwmPropertyScalableGWRTab*>(tabWidget))->updateUI();
                    break;
                case GwmLayerItem::MultiscaleGWR:
                    tabWidget = new GwmPropertyMultiscaleGWRTab(this, static_cast<GwmLayerMultiscaleGWRItem*>(item));
                    (static_cast<GwmPropertyMultiscaleGWRTab*>(tabWidget))->updateUI();
                    break;
                case GwmLayerItem::CollinearityGWR:
                    tabWidget = new GwmPropertyCollinearityGWRTab(this, static_cast<GwmLayerCollinearityGWRItem*>(item));
                    (static_cast<GwmPropertyCollinearityGWRTab*>(tabWidget))->updateUI();
                    break;
                case GwmLayerItem::GeneralizedGWR:
                    tabWidget = new GwmPropertyGGWRTab(this, static_cast<GwmLayerGGWRItem*>(item));
                    (static_cast<GwmPropertyGGWRTab*>(tabWidget))->updateUI();
                    break;
                case GwmLayerItem::GTWR:
                    tabWidget = new GwmPropertyGTWRTab(this, static_cast<GwmLayerGTWRItem*>(item));
                    (static_cast<GwmPropertyGTWRTab*>(tabWidget))->updateUI();
                    break;
                case GwmLayerItem::GWSS:
                    if(static_cast<GwmLayerGWSSItem*>(item)->getType()==2){
                        tabWidget = new GwmPropertyGWCorrelationsTab(this, static_cast<GwmLayerGWSSItem*>(item));
                        (static_cast<GwmPropertyGWCorrelationsTab*>(tabWidget))->updateUI();
                    }
                    else{
                    tabWidget = new GwmPropertyGWSSTab(this, static_cast<GwmLayerGWSSItem*>(item));
                    (static_cast<GwmPropertyGWSSTab*>(tabWidget))->updateUI();
                    }
                    break;
                case GwmLayerItem::GWPCA:
                    tabWidget = new GwmPropertyGWPCATab(this, static_cast<GwmLayerGWPCAItem*>(item));
                    (static_cast<GwmPropertyGWPCATab*>(tabWidget))->updateUI();
                    break;
                default:
                    break;
                }
            addTab(tabWidget, tabName);
            currentChanged(indexOf(tabWidget));
            item->tabWidget = tabWidget;
            }
            else if(indexOf(tabWidget) == -1){
                switch (item->itemType())
                {
                case GwmLayerItem::Origin:
                    tabName = ((GwmLayerOriginItem*)item)->parentItem()->text();
                    break;
                case GwmLayerItem::Group:
                    break;
                case GwmLayerItem::GWR:
                    (static_cast<GwmPropertyGWRTab*>(tabWidget))->updateUI();
                    break;
                case GwmLayerItem::ScalableGWR:
                    (static_cast<GwmPropertyScalableGWRTab*>(tabWidget))->updateUI();
                    break;
                case GwmLayerItem::MultiscaleGWR:
                    (static_cast<GwmPropertyMultiscaleGWRTab*>(tabWidget))->updateUI();
                    break;
                case GwmLayerItem::CollinearityGWR:
                    (static_cast<GwmPropertyCollinearityGWRTab*>(tabWidget))->updateUI();
                    break;
                case GwmLayerItem::GeneralizedGWR:
                    (static_cast<GwmPropertyGGWRTab*>(tabWidget))->updateUI();
                    break;
                case GwmLayerItem::GTWR:
                    (static_cast<GwmPropertyGTWRTab*>(tabWidget))->updateUI();
                    break;
                case GwmLayerItem::GWSS:
                    (static_cast<GwmPropertyGWSSTab*>(tabWidget))->updateUI();
                    break;
                case GwmLayerItem::GWPCA:
                    (static_cast<GwmPropertyGWPCATab*>(tabWidget))->updateUI();
                    break;
                default:
                    break;
                }
               addTab(tabWidget, tabName);
               currentChanged(indexOf(tabWidget));
            }
            else{
                currentChanged(indexOf(tabWidget));
            }
//            addTab(tabWidget, tabName);
            // 默认页
            manageDefaultTab();
        }
    }
}
