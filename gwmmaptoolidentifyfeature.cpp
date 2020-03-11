#include "gwmmaptoolidentifyfeature.h"

GwmMapToolIdentifyFeature::GwmMapToolIdentifyFeature(QgsMapCanvas* mapCanvas)
    : QgsMapToolIdentify(mapCanvas)
    , point0(-1, -1)
    , isMousePressed(false)
{
    rubberBand = new QgsRubberBand(mCanvas, QgsWkbTypes::GeometryType::PolygonGeometry);
    rubberBand->setStrokeColor(QColor(0, 0, 0));
}

void GwmMapToolIdentifyFeature::canvasPressEvent(QgsMapMouseEvent *e)
{
    qDebug() << "Mouse Press (" << e->x() << "," << e->y() << ")";
    point0 = QPoint(e->x(), e->y());
    isMousePressed = true;
}


void GwmMapToolIdentifyFeature::canvasMoveEvent(QgsMapMouseEvent *e)
{
    if (isMousePressed)
    {
        qDebug() << "Mouse Move (" << e->x() << "," << e->y() << ")";
        QgsPointXY corner1 = toMapCoordinates(point0);
        QgsPointXY corner2 = toMapCoordinates(QPoint(point0.x(), e->y()));
        QgsPointXY corner3 = toMapCoordinates(QPoint(e->x(), e->y()));
        QgsPointXY corner4 = toMapCoordinates(QPoint(e->x(), point0.y()));
        if (rubberBand)
        {
            rubberBand->reset(QgsWkbTypes::PolygonGeometry);
            rubberBand->addPoint(corner1, false);
            rubberBand->addPoint(corner2, false);
            rubberBand->addPoint(corner3, false);
            rubberBand->addPoint(corner4, true);
        }
    }
}


void GwmMapToolIdentifyFeature::canvasReleaseEvent(QgsMapMouseEvent *e)
{
    qDebug() << "Mouse Release (" << e->x() << "," << e->y() << ")";
    isMousePressed = false;
    QPoint point(e->x(), e->y());
    IdentifyMode model = QgsMapToolIdentify::LayerSelection;
    if (point0 == point)
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
        else
        {
            QList<QgsMapLayer*> mapLayerSet = mCanvas->layers();
            for (int i = 0; i < mapLayerSet.size(); ++i)
            {
                QgsVectorLayer* layer = (QgsVectorLayer*) mapLayerSet[i];
                layer->removeSelection();
            }
        }
    }
    else
    {
        rubberBand->reset(QgsWkbTypes::PolygonGeometry);
        QgsPointXY mapPoint0 = toMapCoordinates(point0);
        QgsPointXY cur = toMapCoordinates(point);
        QgsRectangle rect(mapPoint0, cur);
        QList<QgsMapLayer*> mapLayerSet = mCanvas->layers();
        for (int i = 0; i < mapLayerSet.size(); ++i)
        {
            QgsVectorLayer* layer = (QgsVectorLayer*) mapLayerSet[i];
            layer->selectByRect(rect);
        }
    }
}
