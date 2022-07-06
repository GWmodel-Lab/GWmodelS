#ifndef GWMLAYERGGWRITEM_H
#define GWMLAYERGGWRITEM_H

#include "gwmlayerbasicgwritem.h"
#include "TaskThread/gwmgeneralizedgwralgorithm.h"
#include "gwmlayerbasicgwritem.h"

class GwmLayerGGWRItem : public GwmLayerBasicGWRItem
{
public:
    GwmLayerGGWRItem(GwmLayerItem* parentItem = nullptr, QgsVectorLayer* vector = nullptr, const GwmGeneralizedGWRAlgorithm* taskThread = nullptr);

    inline virtual GwmLayerItemType itemType() { return GwmLayerItemType::GeneralizedGWR; }

    virtual bool readXml(QDomNode &node) override;
    virtual bool writeXml(QDomNode &node, QDomDocument &doc) override;

    GwmGGWRDiagnostic diagnostic() const;

    GwmGLMDiagnostic GLMdiagnostic() const;

    GwmGeneralizedGWRAlgorithm::Family family() const;

private:
    GwmGeneralizedGWRAlgorithm::Family mFamily;
    GwmGGWRDiagnostic mDiagnostic;
    GwmGLMDiagnostic mGLMDiagnostic;
};

#endif // GWMLAYERGGWRITEM_H
