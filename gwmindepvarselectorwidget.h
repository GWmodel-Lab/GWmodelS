#ifndef GWMINDEPVARSELECTORWIDGET_H
#define GWMINDEPVARSELECTORWIDGET_H

#include <QWidget>
#include <qgsvectorlayer.h>
#include <qstandarditemmodel.h>
#include <Model/gwmlayerattributeitemmodel.h>
#include <Model/gwmlayerattributeitem.h>

namespace Ui {
class GwmIndepVarSelectorWidget;
}

class GwmIndepVarSelectorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GwmIndepVarSelectorWidget(QWidget *parent = nullptr);
    ~GwmIndepVarSelectorWidget();

private:
    Ui::GwmIndepVarSelectorWidget *ui;
    QgsVectorLayer* mLayer = nullptr;
    GwmLayerAttributeItemModel *mIndepVarModel = nullptr;
    GwmLayerAttributeItemModel *mSelectedIndepVarModel = nullptr;

public:
    void layerChanged(QgsVectorLayer* layer = nullptr);
    void onDepVarChanged(QString depVarName);
    void onAddIndepVarBtn();
    void onDelIndepVarBtn();
};

#endif // GWMINDEPVARSELECTORWIDGET_H
