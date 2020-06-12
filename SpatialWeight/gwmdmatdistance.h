#ifndef GWMDMATDISTANCE_H
#define GWMDMATDISTANCE_H

#include <QString>
#include "SpatialWeight/gwmdistance.h"

class GwmDMatDistance : public GwmDistance
{
public:
    GwmDMatDistance();
    GwmDMatDistance(QString dmatFile, int featureCount);
    GwmDMatDistance(const GwmDMatDistance& distance);

    virtual GwmDistance * clone() override
    {
        return new GwmDMatDistance(*this);
    }

    QString dMatFile() const;
    void setDMatFile(const QString &dMatFile);

public:
    virtual vec distance(rowvec params, mat dataPoints) override;

    int featureCount() const;
    void setFeatureCount(int featureCount);

private:
    QString mDMatFile;
    int mFeatureCount;
};

#endif // GWMDMATDISTANCE_H
