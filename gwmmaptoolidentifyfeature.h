#ifndef GWMMAPTOOLIDENTIFYFEATURE_H
#define GWMMAPTOOLIDENTIFYFEATURE_H

#include "prefix.h"

#include <qgsmapcanvas.h>
#include <qgsmaptoolidentify.h>
#include <qgsmapmouseevent.h>

class GwmMapToolIdentifyFeature : public QgsMapToolIdentify
{
public:
    GwmMapToolIdentifyFeature(QgsMapCanvas* mapCanvas);

private:
    int x0;
    int y0;
    bool isMousePressed;

    virtual void canvasPressEvent(QgsMapMouseEvent *e) override;
    virtual void canvasMoveEvent(QgsMapMouseEvent *e) override;
    virtual void canvasReleaseEvent(QgsMapMouseEvent *e) override;
};

#endif // GWMMAPTOOLIDENTIFYFEATURE_H
