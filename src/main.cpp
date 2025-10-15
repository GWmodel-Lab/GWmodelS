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
#include <proj.h>
#include <QDir>
#include "DelimitedText/qgsdelimitedtextprovider.h"
#include "TaskThread/gwmtaskthread.h"
#include <QDebug>

int main(int argc, char *argv[])
{
#if (QT_VERSION >= QT_VERSION_CHECK(5,6,0))
    QgsApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
#endif
    QgsApplication a(argc, argv, true);
    QgsApplication::initQgis();
#ifdef Q_OS_MAC
    QDir appDir(QApplication::applicationDirPath());
    qDebug() << "appDir:" << appDir.absolutePath();
    appDir.cdUp();
    appDir.cd("Resources");
    QgsApplication::setPkgDataPath(appDir.absolutePath());
    appDir.cd("proj");
    std::string proj_resource_dir = appDir.absolutePath().toStdString();
    const char* proj_data_path[] = { proj_resource_dir.c_str() };
#else
    const char* proj_data_path[] = { "./proj" };
    QString pluginDir = "./plugins";
    QgsProviderRegistry::instance(pluginDir);
#endif
    proj_context_set_search_paths(NULL, 1, proj_data_path);
    // 纭繚鍐呯疆鐨勫垎闅旀枃鏈�provider 宸叉敞鍐岋紙鏃犻渶鎻掍欢锛�
    QgsProviderRegistry::instance()->registerProvider(new QgsDelimitedTextProviderMetadata());
    QgsProject::instance()->setCrs(QgsCoordinateReferenceSystem::fromEpsgId(4326));
    qRegisterMetaType<PlotFunction>("PlotFunction");

    GwmApp w;
    w.show();

    return a.exec();
}
