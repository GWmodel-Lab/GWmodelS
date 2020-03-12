#ifndef GWMFEATUREPANEL_H
#define GWMFEATUREPANEL_H

#include <QLabel>
#include <QTreeView>
#include <QStandardItemModel>

class GwmFeaturePanel : public QTreeView
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

    void removeLayerSignal(const QModelIndex &index);

    /**
     * @brief 当行顺序改变时触发的信号
     * @param from 原始的行号
     * @param dest 改变后行号
     */
    void rowOrderChangedSignal(int from, int dest);

    void beginDragDropSignal();
    void endDragDropSignal();

protected:
    /**
     * @brief 重写 mousePressEvent 事件
     * @param event 事件参数
     */
    virtual void mousePressEvent(QMouseEvent *e) override;

    /**
     * @brief 重写 mouseMoveEvent 事件
     * @param event 事件参数
     */
    virtual void mouseMoveEvent(QMouseEvent *e) override;

    /**
     * @brief 重写 dragEnterEvent 事件
     * @param event 事件参数
     */
    virtual void dragEnterEvent(QDragEnterEvent* e) override;

    /**
     * @brief 重写 dragMoveEvent 事件
     * @param event 事件参数
     */
    virtual void dragMoveEvent(QDragMoveEvent *e) override;

    /**
     * @brief 重写 dropEvent 事件
     * @param event 事件参数
     */
    virtual void dropEvent(QDropEvent* e) override;

private:
    QStandardItemModel* mMapModel;

    /**
     * @brief 拖动位置指示器
     */
    QLabel* mIndicator;

    /**
     * @brief 指示是否为合法的拖动对象
     */
    bool mValidPress;

    /**
     * @brief 拖动起点
     */
    QPoint mDragPoint;

    /**
     * @brief 拖动显示的文本
     */
    QString mDragText;

    /**
     * @brief 按下时鼠标相对该行的位置，在拖动过程中保持该相对位置
     */
    QPoint mDragPointAtItem;

    /**
     * @brief 拖动起始行
     */
    int mRowFrom;

    /**
     * @brief 拖动结束行
     */
    int mRowDest;

    /**
     * @brief 计算当前项之前所有项的行高
     * @param index 项的索引
     * @return 总行高
     */
    int sumRowHeight(QModelIndex index);

    /**
     * @brief 执行拖拽
     */
    void doDrag();

    /**
     * @brief 重设序号
     */
    void resetOrder();

    /**
     * @brief 移动所选的行
     * @param from 拖动起始行
     * @param to 拖动结束行
     */
    void doMoveRow(int from, int dest);

    /**
     * @brief 设置界面
     */
    void setupUi();
    void showContextMenu(const QPoint &pos);
    // 发送信号给地图模块(显示图层)
    void showLayer();

    /**
     * @brief 移除图层
     */
    void removeLayer();
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
