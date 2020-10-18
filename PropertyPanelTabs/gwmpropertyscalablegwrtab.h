#ifndef GWMPROPERTYSCALABLEGWRTAB_H
#define GWMPROPERTYSCALABLEGWRTAB_H

#include <QWidget>
#include <QTableWidgetItem>

#include "gwmplot.h"
#include "Model/gwmlayerscalablegwritem.h"

namespace Ui {
class GwmPropertyScalableGWRTab;
}

class GwmPropertyScalableGWRTab : public QWidget
{
    Q_OBJECT

public:
    explicit GwmPropertyScalableGWRTab(QWidget *parent = nullptr, GwmLayerScalableGWRItem* item = nullptr);
    ~GwmPropertyScalableGWRTab();

private:
    Ui::GwmPropertyScalableGWRTab *ui;
    GwmLayerScalableGWRItem* mLayerItem;

public:
    void updateUI();
};
#endif // GWMPROPERTYSCALABLEGWRTAB_H
