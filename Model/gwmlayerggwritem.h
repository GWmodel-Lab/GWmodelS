#ifndef GWMLAYERGGWRITEM_H
#define GWMLAYERGGWRITEM_H

#include "gwmlayergwritem.h"
#include "TaskThread/gwmggwrtaskthread.h"

class GwmLayerGGWRItem : public GwmLayerGWRItem
{
public:
    GwmLayerGGWRItem(GwmLayerItem* parentItem = nullptr, QgsVectorLayer* vector = nullptr, const GwmGGWRTaskThread* taskThread = nullptr);

    inline virtual GwmLayerItemType itemType() { return GwmLayerItemType::GGWR; }

    GwmGGWRDiagnostic diagnostic() const;

    GwmGGWRDiagnostic mDiagnostic;

    GwmGLMDiagnostic mGLMDiagnostic;

    GwmGGWRTaskThread::Family mFamily;

    GwmGLMDiagnostic GLMdiagnostic() const;

    GwmGGWRTaskThread::Family family() const;
};

#endif // GWMLAYERGGWRITEM_H
