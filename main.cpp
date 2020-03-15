#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#include "mainwindow.h"
#include "qgsapplication.h"
#include <QApplication>
#include <qgsproviderregistry.h>

int main(int argc, char *argv[])
{
#if (QT_VERSION >= QT_VERSION_CHECK(5,6,0))
    QgsApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QgsApplication a(argc, argv, true);
    QString pluginDir = "./plugins";
    QgsProviderRegistry::instance(pluginDir);
    MainWindow w;
    w.show();
    return a.exec();
}
