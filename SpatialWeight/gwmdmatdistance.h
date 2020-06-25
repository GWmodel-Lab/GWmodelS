#ifndef GWMDMATDISTANCE_H
#define GWMDMATDISTANCE_H

#include <QString>
#include "SpatialWeight/gwmdistance.h"

class GwmDMatDistance : public GwmDistance
{
public:
    explicit GwmDMatDistance(int total, QString dmatFile);
    GwmDMatDistance(const GwmDMatDistance& distance);

    virtual GwmDistance * clone() override
    {
        return new GwmDMatDistance(*this);
    }

    DistanceType type() override { return DistanceType::DMatDistance; }

    QString dMatFile() const;
    void setDMatFile(const QString &dMatFile);

public:
    virtual vec distance(int focus) override;

private:
    QString mDMatFile;
};

inline QString GwmDMatDistance::dMatFile() const
{
    return mDMatFile;
}

inline void GwmDMatDistance::setDMatFile(const QString &dMatFile)
{
    mDMatFile = dMatFile;
}

#endif // GWMDMATDISTANCE_H
