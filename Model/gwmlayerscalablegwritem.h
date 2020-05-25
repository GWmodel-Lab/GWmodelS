#ifndef GWMLAYERSCALABLEGWRITEM_H
#define GWMLAYERSCALABLEGWRITEM_H

#include "Model/gwmlayergwritem.h"
#include "TaskThread/gwmscalablegwrtaskthread.h"

class GwmLayerScalableGWRItem : public GwmLayerGWRItem
{
    Q_OBJECT
public:
    GwmLayerScalableGWRItem(GwmLayerItem* parentItem = nullptr, QgsVectorLayer* vector = nullptr, const GwmScalableGWRTaskThread* taskThread = nullptr);

    double getCV() const;

    double getScale() const;

    double getPenalty() const;

    int getPolynomial() const;

private:
    int mPolynomial;
    double mCV;
    double mScale;
    double mPenalty;
};

#endif // GWMLAYERSCALABLEGWRITEM_H
