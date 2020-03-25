#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#include "mainwindow.h"
#include "qgsapplication.h"
#include <QApplication>
#include <qgsproviderregistry.h>
#include <qgsproject.h>
#include <qgscoordinatereferencesystem.h>

int main(int argc, char *argv[])
{
#if (QT_VERSION >= QT_VERSION_CHECK(5,6,0))
    QgsApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QgsApplication a(argc, argv, true);
    QgsApplication::initQgis();
    QString pluginDir = "./plugins";
    QgsProviderRegistry::instance(pluginDir);
    QgsProject::instance()->setCrs(QgsCoordinateReferenceSystem::fromEpsgId(4326));
    MainWindow w;
    w.show();
    return a.exec();
}
