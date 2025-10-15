#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#include "gwmapp.h"
#include "qgsapplication.h"
#include <QApplication>
#include <QMetaType>
#include <qgsproviderregistry.h>
#include <qgsproject.h>
#include <qgscoordinatereferencesystem.h>
#include <qgslayoutitemguiregistry.h>
#include "DelimitedText/qgsdelimitedtextprovider.h"

#include "TaskThread/gwmtaskthread.h"

int main(int argc, char *argv[])
{
#if (QT_VERSION >= QT_VERSION_CHECK(5,6,0))
    QgsApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
#endif
    QgsApplication a(argc, argv, true);
    QgsApplication::initQgis();
    QString pluginDir = "./plugins";
    QgsProviderRegistry::instance(pluginDir);
    // 确保内置的分隔文本 provider 已注册（无需插件）
    QgsProviderRegistry::instance()->registerProvider(new QgsDelimitedTextProviderMetadata());
    QgsProject::instance()->setCrs(QgsCoordinateReferenceSystem::fromEpsgId(4326));
    qRegisterMetaType<PlotFunction>("PlotFunction");

    GwmApp w;
    w.show();


    return a.exec();
}
