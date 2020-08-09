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
    int length() const override;

    int rowSize() const;
    void setRowSize(int rowSize);

private:
    QString mDMatFile;

    int mRowSize = 0;
};

inline QString GwmDMatDistance::dMatFile() const
{
    return mDMatFile;
}

inline void GwmDMatDistance::setDMatFile(const QString &dMatFile)
{
    mDMatFile = dMatFile;
}

inline int GwmDMatDistance::length() const
{
    return rowSize();
}

inline int GwmDMatDistance::rowSize() const
{
    return mRowSize;
}

inline void GwmDMatDistance::setRowSize(int rowSize)
{
    mRowSize = rowSize;
}

#endif // GWMDMATDISTANCE_H
