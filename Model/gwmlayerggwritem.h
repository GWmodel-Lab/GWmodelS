#ifndef GWMLAYERGGWRITEM_H
#define GWMLAYERGGWRITEM_H

#include "gwmlayergwritem.h"
#include "TaskThread/gwmggwralgorithm.h"
#include "gwmlayerbasicgwritem.h"

class GwmLayerGGWRItem : public GwmLayerBasicGWRItem
{
public:
    GwmLayerGGWRItem(GwmLayerItem* parentItem = nullptr, QgsVectorLayer* vector = nullptr, const GwmGGWRAlgorithm* taskThread = nullptr);

    inline virtual GwmLayerItemType itemType() { return GwmLayerItemType::GGWR; }

    GwmGGWRDiagnostic diagnostic() const;

    GwmGGWRDiagnostic mDiagnostic;

    GwmGLMDiagnostic mGLMDiagnostic;

    GwmGGWRAlgorithm::Family mFamily;

    GwmGLMDiagnostic GLMdiagnostic() const;

    GwmGGWRAlgorithm::Family family() const;
};

#endif // GWMLAYERGGWRITEM_H
