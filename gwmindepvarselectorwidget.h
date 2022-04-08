#ifndef GWMINDEPVARSELECTORWIDGET_H
#define GWMINDEPVARSELECTORWIDGET_H

#include <QWidget>
#include <qgsvectorlayer.h>
#include <qstandarditemmodel.h>
#include <Model/gwmlayerattributeitemmodel.h>
#include <Model/gwmlayerattributeitem.h>

#include <Model/gwmvariableitemmodel.h>

namespace Ui {
class GwmIndepVarSelectorWidget;
}

class GwmIndepVarSelectorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GwmIndepVarSelectorWidget(QWidget *parent = nullptr);
    ~GwmIndepVarSelectorWidget();

signals:
    void selectedIndepVarChangedSignal();

private:
    Ui::GwmIndepVarSelectorWidget *ui;
    QgsVectorLayer* mLayer = nullptr;
    GwmVariableItemModel *mIndepVarModel = nullptr;


public:
    void layerChanged(QgsVectorLayer* layer = nullptr);
    void onDepVarChanged(QString depVarName);
    void onAddIndepVarBtn();
    void onDelIndepVarBtn();
    GwmVariableItemModel *indepVarModel() const;
    GwmVariableItemModel *selectedIndepVarModel() const;
    void onIndepVarChanged(GwmVariableItemModel *mXSelectedIndepVarModel);
    GwmVariableItemModel *mSelectedIndepVarModel = nullptr;

private:
    bool isNumeric(QVariant::Type type);
};

#endif // GWMINDEPVARSELECTORWIDGET_H
