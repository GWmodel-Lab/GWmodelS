#ifndef GWMMAPTOOLIDENTIFYFEATURE_H
#define GWMMAPTOOLIDENTIFYFEATURE_H

#include "prefix.h"

#include <qgsmapcanvas.h>
#include <qgsmaptoolidentify.h>
#include <qgsmapmouseevent.h>
#include <qgsrubberband.h>

class GwmMapToolIdentifyFeature : public QgsMapToolIdentify
{
public:
    GwmMapToolIdentifyFeature(QgsMapCanvas* mapCanvas);

private:
    QPoint point0;
    bool isMousePressed;
    QgsRubberBand* rubberBand;


    virtual void canvasPressEvent(QgsMapMouseEvent *e) override;
    virtual void canvasMoveEvent(QgsMapMouseEvent *e) override;
    virtual void canvasReleaseEvent(QgsMapMouseEvent *e) override;
};

#endif // GWMMAPTOOLIDENTIFYFEATURE_H
