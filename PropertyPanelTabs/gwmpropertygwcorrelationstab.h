#ifndef GWMPROPERTYGWCORRELATIONSTAB_H
#define GWMPROPERTYGWCORRELATIONSTAB_H

#include <QWidget>

#include "Model/gwmlayergwssitem.h"
#include <QTableWidgetItem>
#include <QTableWidget>
#include "Model/gwmpropertygwcorrelationsparameterspecifieditemmodel.h"

namespace Ui {
class GwmPropertyGWCorrelationsTab;
}

class GwmPropertyGWCorrelationsTab : public QWidget
{
    Q_OBJECT

public:
    explicit GwmPropertyGWCorrelationsTab(QWidget *parent = nullptr, GwmLayerGWSSItem *item = nullptr);
    ~GwmPropertyGWCorrelationsTab();

public:
    void updateUI();

private:
    Ui::GwmPropertyGWCorrelationsTab *ui;
    GwmLayerGWSSItem* mLayerItem;
    GwmPropertyGWCorrelationsParameterSpecifiedItemModel* mParameterSpecifiedModel = nullptr;
};

#endif // GWMPROPERTYGWCORRELATIONSTAB_H
