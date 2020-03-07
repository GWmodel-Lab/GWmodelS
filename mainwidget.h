#ifndef MAINLAYOUT_H
#define MAINLAYOUT_H

#include <QWidget>
#include <gwmodeltoolbar.h>
#include <gwmodelmappanel.h>

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
    GWmodelToolbar* toolBar;
    QWidget* mainZone;
    GWmodelMapPanel* mapPanel;
    QTreeView* featurePanel;
    QTabWidget* propertyPanel;

    QStandardItemModel* mapModel;
public slots:
    void openFileImportShapefile();
    void openFileImportJson();
    void openFileImportCsv();
    // 要素区右键显示菜单
    void showContextMenu(const QPoint &pos);
    // 显示图层
    void showLayer();
private:
    void createMainZone();
    void createFeaturePanel();
    void createPropertyPanel();
    // 官网示例函数
    void ShowContext(const QModelIndex &index);
    // 要素区右键显示菜单信号函数
    void customContextMenuRequested(const QPoint &pos);
    // 切换图层信号函数
    void showLayerSig(const QModelIndex &index);
    // 发送信号
    void sendDataSig(const QModelIndex &index);
};

#endif // MAINLAYOUT_H
