#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#include "mainwindow.h"
#include "qgsapplication.h"
#include <QApplication>
#include <QMetaType>
#include <qgsproviderregistry.h>
#include <qgsproject.h>
#include <qgscoordinatereferencesystem.h>

#include "TaskThread/gwmtaskthread.h"

#define QT_PLUGINS_DIR "D:/OSGeo4W64/apps/qgis-dev/qtplugins;D:/OSGeo4W64/apps/Qt5/plugins"

int main(int argc, char *argv[])
{
#if (QT_VERSION >= QT_VERSION_CHECK(5,6,0))
    QgsApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QgsApplication a(argc, argv, true);


    QStringList libPaths( QCoreApplication::libraryPaths() );

    QgsDebugMsgLevel( QStringLiteral( "Initial macOS/UNIX QCoreApplication::libraryPaths: %1" )
                      .arg( libPaths.join( " " ) ), 4 );

    // Strip all critical paths that should always be prepended
    if ( libPaths.removeAll( QDir::cleanPath( QgsApplication::pluginPath() ) ) )
    {
      QgsDebugMsgLevel( QStringLiteral( "QgsApplication::pluginPath removed from initial libraryPaths" ), 4 );
    }
    if ( libPaths.removeAll( QCoreApplication::applicationDirPath() ) )
    {
      QgsDebugMsgLevel( QStringLiteral( "QCoreApplication::applicationDirPath removed from initial libraryPaths" ), 4 );
    }
    // Prepend path, so a standard Qt bundle directory is parsed
    QgsDebugMsgLevel( QStringLiteral( "Prepending QCoreApplication::applicationDirPath to libraryPaths" ), 4 );
    libPaths.prepend( QCoreApplication::applicationDirPath() );

    // Check if we are running in a 'release' app bundle, i.e. contains copied-in
    // standard Qt-specific plugin subdirectories (ones never created by QGIS, e.g. 'sqldrivers' is).
    // Note: bundleclicked(...) is inadequate to determine which *type* of bundle was opened, e.g. release or build dir.
    // An app bundled with QGIS_MACAPP_BUNDLE > 0 is considered a release bundle.
    QString  relLibPath( QDir::cleanPath( QCoreApplication::applicationDirPath().append( "/../PlugIns" ) ) );
    // Note: relLibPath becomes the defacto QT_PLUGINS_DIR of a release app bundle
    if ( QFile::exists( relLibPath + QStringLiteral( "/imageformats" ) )
         && QFile::exists( relLibPath + QStringLiteral( "/codecs" ) ) )
    {
      // We are in a release app bundle.
      // Strip QT_PLUGINS_DIR because it will crash a launched release app bundle, since
      // the appropriate Qt frameworks and plugins have been copied into the bundle.
      if ( libPaths.removeAll( QT_PLUGINS_DIR ) )
      {
        QgsDebugMsgLevel( QStringLiteral( "QT_PLUGINS_DIR removed from initial libraryPaths" ), 4 );
      }
      // Prepend the Plugins path, so copied-in Qt plugin bundle directories are parsed.
      QgsDebugMsgLevel( QStringLiteral( "Prepending <bundle>/Plugins to libraryPaths" ), 4 );
      libPaths.prepend( relLibPath );

      // TODO: see if this or another method can be used to avoid QCA's install prefix plugins
      //       from being parsed and loaded (causes multi-Qt-loaded errors when bundled Qt should
      //       be the only one loaded). QCA core (> v2.1.3) needs an update first.
      //setenv( "QCA_PLUGIN_PATH", relLibPath.toUtf8().constData(), 1 );
    }
    else
    {
      // We are either running from build dir bundle, or launching Mach-O binary directly.  //#spellok
      // Add system Qt plugins, since they are not bundled, and not always referenced by default.
      // An app bundled with QGIS_MACAPP_BUNDLE = 0 will still have Plugins/qgis in it.
      // Note: Don't always prepend.
      //       User may have already defined it in QT_PLUGIN_PATH in a specific order.
      if ( !libPaths.contains( QT_PLUGINS_DIR ) )
      {
        QgsDebugMsgLevel( QStringLiteral( "Prepending QT_PLUGINS_DIR to libraryPaths" ), 4 );
        libPaths.prepend( QT_PLUGINS_DIR );
      }
    }

    QgsDebugMsgLevel( QStringLiteral( "Prepending QgsApplication::pluginPath to libraryPaths" ), 4 );
    libPaths.prepend( QDir::cleanPath( QgsApplication::pluginPath() ) );

    // Redefine library search paths.
    QCoreApplication::setLibraryPaths( libPaths );

    QgsDebugMsgLevel( QStringLiteral( "Rewritten macOS QCoreApplication::libraryPaths: %1" )
                      .arg( QCoreApplication::libraryPaths().join( " " ) ), 4 );
    QgsApplication::initQgis();
    QString pluginDir = "./plugins";
    QgsProviderRegistry::instance(pluginDir);
    QgsProject::instance()->setCrs(QgsCoordinateReferenceSystem::fromEpsgId(4326));

    GwmGWRTaskThread::initUnitDict();
    qRegisterMetaType<PlotFunction>("PlotFunction");

    MainWindow w;
    w.show();
    return a.exec();
}
