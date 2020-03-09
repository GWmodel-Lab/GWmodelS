#include "gwmpropertypanel.h"
#include "PropertyPanelTabs/gwmpropertystatisticstab.h"

GwmPropertyPanel::GwmPropertyPanel(QWidget *parent, QStandardItemModel* model) :
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

void addModelItem(QStandardItem* item, QString property, QString value)
{
    QStandardItem* subitem = new QStandardItem(property);
    item->appendRow(subitem);
    item->setChild(subitem->index().row(), 1, new QStandardItem(value));
}

void GwmPropertyPanel::addStatisticTab(QModelIndex index, QgsVectorLayer* layer)
{
    QStandardItemModel* model = new QStandardItemModel(0, 2);

    // 要素
    QStandardItem* geometryItem = new QStandardItem(QStringLiteral("Feature"));
    model->appendRow(geometryItem);
    addModelItem(geometryItem, QStringLiteral("Count"), QString("%1").arg(layer->featureCount()));
    addModelItem(geometryItem, QStringLiteral("Geometry"), QgsWkbTypes::geometryDisplayString(layer->geometryType()));
    QgsRectangle layerExtent = layer->extent();
    addModelItem(geometryItem, QStringLiteral("Min X"), QString("%1").arg(layer->extent().xMinimum(), 0, 'g', 12));
    addModelItem(geometryItem, QStringLiteral("Max X"), QString("%1").arg(layer->extent().xMaximum(), 0, 'g', 12));
    addModelItem(geometryItem, QStringLiteral("Min Y"), QString("%1").arg(layer->extent().yMinimum(), 0, 'g', 12));
    addModelItem(geometryItem, QStringLiteral("Max Y"), QString("%1").arg(layer->extent().yMaximum(), 0, 'g', 12));


    // 属性列表
    QStandardItem* layerFieldsItem = new QStandardItem("Fields");
    model->appendRow(layerFieldsItem);
    QgsFields fields = layer->fields();
    for (auto field = fields.begin(); field != fields.end(); field++) {
        addModelItem(layerFieldsItem, field->name(), field->typeName());
    }

    // 空间参考
    QStandardItem* projectionItem = new QStandardItem("Spatial Reference");
    model->appendRow(projectionItem);
    QgsCoordinateReferenceSystem crs = layer->crs();
    addModelItem(projectionItem, QStringLiteral("ID"), crs.authid());
    addModelItem(projectionItem, QStringLiteral("Type"), crs.isGeographic() ? QStringLiteral("Geographic") : QStringLiteral("Projected"));
    addModelItem(projectionItem, QStringLiteral("WKT"), crs.toWkt());

    // 创建标签页
    QStandardItem* item = mapModel->itemFromIndex(index);
    GwmPropertyStatisticsTab* tab = new GwmPropertyStatisticsTab(this, model);
    addTab(tab, item->text());

    // 默认页
    manageDefaultTab();
}
