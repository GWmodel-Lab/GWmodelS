#ifndef GWMODELMAPPANEL_H
#define GWMODELMAPPANEL_H

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#include <QWidget>
#include <QStandardItemModel>
#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>

class GwmMapPanel : public QWidget
{
    Q_OBJECT

public:
    explicit GwmMapPanel(QWidget *parent = nullptr, QStandardItemModel* model = new QStandardItemModel);
    ~GwmMapPanel();

private:
    QgsMapCanvas* mapCanvas;
    QStandardItemModel* mapModel;
    QList<QgsMapLayer*> mapLayerSet;

    void onMapItemInserted(const QModelIndex &parent, int first, int last);
public slots:
    // 显示图层
    void receiveShowLayer(const QModelIndex &index);
    // 缩放至图层
    void receiveZoomLayer(const QModelIndex &index);
    // 属性表
    void receiveAttribute(const QModelIndex &index);
    // 投影到坐标系
    void receiveProj(const QModelIndex &index);
    // 符号
    void receiveSymbol(const QModelIndex &index);
    // 导出shp
    void receiveShp(const QModelIndex &index);
    // 导出GeoJSON
    void receiveGeoJson(const QModelIndex &index);
    // 导出Excel
    void receiveExcel(const QModelIndex &index);
    // 导出Csv
    void receiveCsv(const QModelIndex &index);
};

#endif // GWMODELMAPPANEL_H
