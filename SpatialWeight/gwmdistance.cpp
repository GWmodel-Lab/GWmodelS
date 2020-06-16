#include "gwmdistance.h"

GwmEnumValueNameMapper<GwmDistance::DistanceType> GwmDistance::TypeNameMapper =
{
    std::make_pair(GwmDistance::DistanceType::CRSDistance, "CRSDistance"),
    std::make_pair(GwmDistance::DistanceType::MinkwoskiDistance, "MinkwoskiDistance"),
    std::make_pair(GwmDistance::DistanceType::DMatDistance, "DMatDistance")
};

double GwmDistance::maxDistance(int n)
{
    double maxD = 0.0;
    for (int i = 0; i < n; i++)
    {
        double d = max(distance(i));
        maxD = d > maxD ? d : maxD;
    }
    return maxD;
}

double GwmDistance::minDistance(int n)
{
    double minD = DBL_MAX;
    for (int i = 0; i < n; i++)
    {
        double d = min(distance(i));
        minD = d < minD ? d : minD;
    }
    return minD;
}
