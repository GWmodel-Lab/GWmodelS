#include "gwmpropertystatisticstab.h"
#include "ui_gwmpropertystatisticstab.h"

#include "utils.h"

GwmPropertyStatisticsTab::GwmPropertyStatisticsTab(QWidget *parent, GwmLayerOriginItem* layerItem) :
    QWidget(parent),
    ui(new Ui::GwmPropertyStatisticsTab),
    mLayerItem(layerItem)
{
    ui->setupUi(this);
    createModel();
    ui->propertyView->setModel(mModel);
    ui->propertyView->setEditTriggers(QAbstractItemView::NoEditTriggers);
//    mLayerItem->setHorizontalHeaderLabels(QStringList() << "Property" << "Value");
}

GwmPropertyStatisticsTab::~GwmPropertyStatisticsTab()
{
    delete ui;
}

void GwmPropertyStatisticsTab::createModel()
{
    QgsVectorLayer* layer = mLayerItem->layer();
    mModel = new QStandardItemModel(0, 2);
    mModel->setHorizontalHeaderLabels(QStringList() << "Property" << "Value");

    // 要素
    QStandardItem* geometryItem = new QStandardItem(QStringLiteral("Feature"));
    mModel->appendRow(geometryItem);
    addModelItem(geometryItem, QStringLiteral("Count"), QString("%1").arg(layer->featureCount()));
    addModelItem(geometryItem, QStringLiteral("Geometry"), QgsWkbTypes::geometryDisplayString(layer->geometryType()));
    QgsRectangle layerExtent = layer->extent();
    addModelItem(geometryItem, QStringLiteral("Min X"), QString("%1").arg(layer->extent().xMinimum(), 0, 'g', 12));
    addModelItem(geometryItem, QStringLiteral("Max X"), QString("%1").arg(layer->extent().xMaximum(), 0, 'g', 12));
    addModelItem(geometryItem, QStringLiteral("Min Y"), QString("%1").arg(layer->extent().yMinimum(), 0, 'g', 12));
    addModelItem(geometryItem, QStringLiteral("Max Y"), QString("%1").arg(layer->extent().yMaximum(), 0, 'g', 12));


    // 属性列表
    QStandardItem* layerFieldsItem = new QStandardItem("Fields");
    mModel->appendRow(layerFieldsItem);
    QgsFields fields = layer->fields();
    for (auto field = fields.begin(); field != fields.end(); field++) {
        addModelItem(layerFieldsItem, field->name(), field->typeName());
    }

    // 空间参考
    QStandardItem* projectionItem = new QStandardItem("Spatial Reference");
    mModel->appendRow(projectionItem);
    QgsCoordinateReferenceSystem crs = layer->crs();
    addModelItem(projectionItem, QStringLiteral("ID"), crs.authid());
    addModelItem(projectionItem, QStringLiteral("Type"), crs.isGeographic() ? QStringLiteral("Geographic") : QStringLiteral("Projected"));
    addModelItem(projectionItem, QStringLiteral("WKT"), crs.toWkt());
}
