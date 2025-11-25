#ifndef GWMPROPERTYGWCORRELATIONTAB_H
#define GWMPROPERTYGWCORRELATIONTAB_H

#include <QWidget>

#include "Model/gwmlayergwcorrelationitem.h"
#include <QTableWidgetItem>
#include <QTableWidget>
#include "Model/gwmpropertygwcorrelationsparameterspecifieditemmodel.h"

namespace Ui {
class GwmPropertyGWCorrelationTab;
}

class GwmPropertyGWCorrelationTab : public QWidget
{
    Q_OBJECT

public:
    explicit GwmPropertyGWCorrelationTab(QWidget *parent = nullptr, GwmLayerGWCorrelationItem *item = nullptr);
    ~GwmPropertyGWCorrelationTab();

public:
    void updateUI();

private:
    Ui::GwmPropertyGWCorrelationTab *ui;
    GwmLayerGWCorrelationItem* mLayerItem;
    GwmPropertyGWCorrelationsParameterSpecifiedItemModel* mParameterSpecifiedModel = nullptr;
};

#endif // GWMPROPERTYGWCORRELATIONTAB_H
