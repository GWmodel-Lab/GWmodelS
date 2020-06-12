#include "gwmdistance.h"

GwmEnumValueNameMapper<GwmDistance::DistanceType> GwmDistance::TypeNameMapper =
{
    std::make_pair(GwmDistance::DistanceType::CRSDistance, "CRSDistance"),
    std::make_pair(GwmDistance::DistanceType::MinkwoskiDistance, "MinkwoskiDistance"),
    std::make_pair(GwmDistance::DistanceType::DMatDistance, "DMatDistance")
};
