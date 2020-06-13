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
    virtual vec distance(const rowvec& params, const mat& dataPoints) override;

    int featureCount() const;
    void setFeatureCount(int featureCount);

private:
    QString mDMatFile;
    int mFeatureCount;
};

inline int GwmDMatDistance::featureCount() const
{
    return mFeatureCount;
}

inline void GwmDMatDistance::setFeatureCount(int featureCount)
{
    mFeatureCount = featureCount;
}

inline QString GwmDMatDistance::dMatFile() const
{
    return mDMatFile;
}

inline void GwmDMatDistance::setDMatFile(const QString &dMatFile)
{
    mDMatFile = dMatFile;
}

#endif // GWMDMATDISTANCE_H
