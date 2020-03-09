#ifndef MAINLAYOUT_H
#define MAINLAYOUT_H

#include <QWidget>
#include <gwmtoolbar.h>
#include <gwmmappanel.h>

//namespace Ui {
//class MainLayout;
//}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();


public:
    QVBoxLayout* mainLayout;
    GwmToolbar* toolBar;
    QWidget* mainZone;
    GwmMapPanel* mapPanel;
    QTreeView* featurePanel;
    QTabWidget* propertyPanel;

    QStandardItemModel* mapModel;
public slots:
    void openFileImportShapefile();
    void openFileImportJson();
    void openFileImportCsv();
    // 要素区右键显示菜单
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
private:
    void createMainZone();
    void createFeaturePanel();
    void createPropertyPanel();
    // 官网示例函数
    void ShowContext(const QModelIndex &index);
    // 要素区右键显示菜单信号函数
    void customContextMenuRequested(const QPoint &pos);
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
};

#endif // MAINLAYOUT_H
