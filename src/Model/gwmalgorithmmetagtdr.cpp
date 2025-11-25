#include "gwmalgorithmmetagtdr.h"

#include <QTranslator>

using namespace gwm;

bool GwmAlgorithmMetaGTDR::validate(QString &error) const
{
    if (layer == nullptr)
    {
        error = QTranslator::tr("Layer is missing.");
        return false;
    }

    if (independentVariables.size() <= 0)
    {
        error = QTranslator::tr("No variable selected.");
        return false;
    }

    if (weightType == Weight::WeightType::BandwidthWeight)
    {
        if (weightBandwidthSize == 0)
        {
            error = QTranslator::tr("Bandwidth size is too small.");
            return false;
        }
    }

    return true;
}