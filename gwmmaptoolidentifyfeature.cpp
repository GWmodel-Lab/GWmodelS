#include "gwmmaptoolidentifyfeature.h"

GwmMapToolIdentifyFeature::GwmMapToolIdentifyFeature(QgsMapCanvas* mapCanvas)
    : QgsMapToolIdentify(mapCanvas)
    , x0(-1)
    , y0(-1)
    , isMousePressed(false)
{

}

void GwmMapToolIdentifyFeature::canvasPressEvent(QgsMapMouseEvent *e)
{
//    qDebug() << "Mouse Press (" << e->x() << "," << e->y() << ")";
    x0 = e->x();
    y0 = e->y();
    isMousePressed = true;
}


void GwmMapToolIdentifyFeature::canvasMoveEvent(QgsMapMouseEvent *e)
{
//    if (isMousePressed)
//    {
//        qDebug() << "Mouse Move (" << e->x() << "," << e->y() << ")";
//    }
}


void GwmMapToolIdentifyFeature::canvasReleaseEvent(QgsMapMouseEvent *e)
{
//    qDebug() << "Mouse Release (" << e->x() << "," << e->y() << ")";
    int x = e->x(), y = e->y();
    if (isMousePressed)
    {
        isMousePressed = false;
    }
    IdentifyMode model = QgsMapToolIdentify::LayerSelection;
    if (x0 == x && y0 == y)
    {
        QList<IdentifyResult> results = QgsMapToolIdentify::identify(e->x(), e->y(), model);
        if (!results.isEmpty())
        {
            QMap<QgsVectorLayer*, QgsFeatureIds> selectionMap;
            for (auto result = results.begin(); result != results.end(); result++)
            {
                QgsVectorLayer* layer = (QgsVectorLayer*) result->mLayer;
                if (!selectionMap.contains(layer))
                {
                    selectionMap[layer] = QgsFeatureIds();
                }
                selectionMap[layer] = selectionMap[layer] << result->mFeature.id();
            }
            for (auto iterator = selectionMap.begin(); iterator != selectionMap.end(); iterator++)
            {
                QgsVectorLayer* layer = iterator.key();
                layer->selectByIds(iterator.value());
            }
        }
    }

}
