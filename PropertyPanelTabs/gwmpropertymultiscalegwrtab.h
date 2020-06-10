#ifndef GWMPROPERTYMULTISCALEGWRTAB_H
#define GWMPROPERTYMULTISCALEGWRTAB_H

#include <QWidget>
#include <QTableWidgetItem>

#include "gwmplot.h"
#include "Model/gwmlayermultiscalegwritem.h"
#include "Model/gwmpropertymultiscaleparameterspecifieditemmodel.h"

namespace Ui {
class GwmPropertyMultiscaleGWRTab;
}

class GwmPropertyMultiscaleGWRTab : public QWidget
{
    Q_OBJECT

public:
    static QMap<GwmGWRTaskThread::KernelFunction, QString> kernelFunctionNameDict;
    static QMap<GwmGWRTaskThread::BandwidthType, QString> bandwidthTypeNameDict;

public:
    explicit GwmPropertyMultiscaleGWRTab(QWidget *parent = nullptr, GwmLayerMultiscaleGWRItem* item = nullptr);
    ~GwmPropertyMultiscaleGWRTab();

private:
    Ui::GwmPropertyMultiscaleGWRTab *ui;
    GwmLayerMultiscaleGWRItem* mLayerItem;
    GwmPropertyMultiscaleParameterSpecifiedItemModel* mParameterSpecifiedModel = nullptr;

public:
    void updateUI();
};


#endif // GWMPROPERTYGWRTAB_H
