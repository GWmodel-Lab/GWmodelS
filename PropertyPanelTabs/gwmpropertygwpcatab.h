#ifndef GWMPROPERTYGWPCATAB_H
#define GWMPROPERTYGWPCATAB_H

#include <QWidget>
#include <QTableWidgetItem>

#include "gwmplot.h"
#include "Model/gwmlayergwpcaitem.h"

namespace Ui {
class GwmPropertyGWPCATab;
}


class GwmPropertyGWPCATab : public QWidget
{
    Q_OBJECT

public:

public:
    explicit GwmPropertyGWPCATab(QWidget *parent = nullptr, GwmLayerGWPCAItem* item = nullptr);
    ~GwmPropertyGWPCATab();

private:
    Ui::GwmPropertyGWPCATab *ui;
    GwmLayerGWPCAItem* mLayerItem;


public:
    void updateUI();

    GwmPlot* mBandwidthSelPlot;
};


#endif // GWMPROPERTYGWRTAB_H
