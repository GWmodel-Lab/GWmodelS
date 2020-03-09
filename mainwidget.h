#ifndef MAINLAYOUT_H
#define MAINLAYOUT_H

#include <QWidget>
#include <gwmtoolbar.h>
#include <gwmmappanel.h>
#include <gwmfeaturepanel.h>

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
    QWidget* mainZone;
    GwmToolbar* toolBar;
    GwmMapPanel* mapPanel;
    GwmFeaturePanel* featurePanel;
    QTabWidget* propertyPanel;

    QStandardItemModel* mapModel;

public slots:
    void openFileImportShapefile();
    void openFileImportJson();
    void openFileImportCsv();

private:
    void createMainZone();
    void createFeaturePanel();
    void createPropertyPanel();
    // 官网示例函数
    void ShowContext(const QModelIndex &index);
    // 要素区右键显示菜单信号函数
    void customContextMenuRequested(const QPoint &pos);
};

#endif // MAINLAYOUT_H
