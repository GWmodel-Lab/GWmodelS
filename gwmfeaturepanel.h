#ifndef GWMFEATUREPANEL_H
#define GWMFEATUREPANEL_H

#include <QTreeView>
#include <QStandardItemModel>

class GwmFeaturePanel : public QWidget
{
    Q_OBJECT

public:
    explicit GwmFeaturePanel(QWidget *parent = nullptr, QStandardItemModel* model = new QStandardItemModel);
    ~GwmFeaturePanel();

signals:
    // 显示图层
    void sendDataSigShowLayer(const QModelIndex &index);
    // 缩放至图层
    void sendDataSigZoomLayer(const QModelIndex &index);
    // 属性表
    void sendDataSigAttributeTable(const QModelIndex &index);
    // 投影到坐标系
    void sendDataSigProj(const QModelIndex &index);
    // 符号
    void sendDataSigSymbol(const QModelIndex &index);
    // 导出shp
    void sendDataSigEsriShp(const QModelIndex &index);
    // 导出GeoJSON
    void sendDataSigGeoJson(const QModelIndex &index);
    // 导出Excel
    void sendDataSigExcel(const QModelIndex &index);
    // 导出Csv
    void sendDataSigCsv(const QModelIndex &index);
    // 显示属性
    void showLayerPropertySignal(const QModelIndex &index);

private:
    QStandardItemModel* mapModel;
    QTreeView* featurePanel;

    void setupUi();
    void showContextMenu(const QPoint &pos);
    // 发送信号给地图模块(显示图层)
    void showLayer();
    // 发送信号(缩放至图层)
    void zoomLayer();
    // 发送信号(属性表)
    void attributeTable();
    // 发送信号(投影到坐标系)
    void proj();
    // 发送信号(符号)
    void symbol();
    // 发送信号(导出shp)
    void esrishp();
    // 发送信号(导出GeoJSON)
    void geojson();
    // 发送信号(导出Excel)
    void excel();
    // 发送信号(导出csv)
    void csv();
    // 发出信号(显示属性)
    void layerProperty();
};

#endif // GWMFEATUREPANEL_H
