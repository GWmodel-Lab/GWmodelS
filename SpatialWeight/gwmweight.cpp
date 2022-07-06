#include "gwmweight.h"

GwmEnumValueNameMapper<GwmWeight::WeightType> GwmWeight::TypeNameMapper = {
    std::make_pair(GwmWeight::WeightType::BandwidthWeight, "BandwidthWeight")
};
