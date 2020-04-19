#ifndef GWMPROPERTYGWRTAB_H
#define GWMPROPERTYGWRTAB_H

#include <QWidget>
#include "Model/gwmlayergwritem.h"

namespace Ui {
class GwmPropertyGWRTab;
}

class GwmPropertyGWRTab : public QWidget
{
    Q_OBJECT

public:
    static QMap<GwmGWRTaskThread::KernelFunction, QString> kernelFunctionNameDict;
    static QMap<GwmGWRTaskThread::BandwidthType, QString> bandwidthTypeNameDict;

public:
    explicit GwmPropertyGWRTab(QWidget *parent = nullptr, GwmLayerGWRItem* item = nullptr);
    ~GwmPropertyGWRTab();

private:
    Ui::GwmPropertyGWRTab *ui;
    GwmLayerGWRItem* mLayerItem;

private:
    void updateUI();
};

#endif // GWMPROPERTYGWRTAB_H
