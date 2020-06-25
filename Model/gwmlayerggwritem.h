#ifndef GWMLAYERGGWRITEM_H
#define GWMLAYERGGWRITEM_H

#include "gwmlayergwritem.h"
#include "TaskThread/gwmgeneralizedgwralgorithm.h"
#include "gwmlayerbasicgwritem.h"

class GwmLayerGGWRItem : public GwmLayerBasicGWRItem
{
public:
    GwmLayerGGWRItem(GwmLayerItem* parentItem = nullptr, QgsVectorLayer* vector = nullptr, const GwmGeneralizedGWRAlgorithm* taskThread = nullptr);

    inline virtual GwmLayerItemType itemType() { return GwmLayerItemType::GeneralizedGWR; }

    GwmGGWRDiagnostic diagnostic() const;

    GwmGGWRDiagnostic mDiagnostic;

    GwmGLMDiagnostic mGLMDiagnostic;

    GwmGeneralizedGWRAlgorithm::Family mFamily;

    GwmGLMDiagnostic GLMdiagnostic() const;

    GwmGeneralizedGWRAlgorithm::Family family() const;
};

#endif // GWMLAYERGGWRITEM_H
