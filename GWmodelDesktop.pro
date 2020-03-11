QT       += core gui xml svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    gwmattributetableview.cpp \
    gwmfeaturepanel.cpp \
    PropertyPanelTabs/gwmpropertydefaulttab.cpp \
    PropertyPanelTabs/gwmpropertystatisticstab.cpp \
    gwmmaptoolidentifyfeature.cpp \
    gwmpropertypanel.cpp \
    gwmsymbolwindow.cpp \
    gwmtoolbar.cpp \
    main.cpp \
    mainwidget.cpp \
    mainwindow.cpp

HEADERS += \
    gwmattributetableview.h \
    gwmfeaturepanel.h \
    PropertyPanelTabs/gwmpropertydefaulttab.h \
    PropertyPanelTabs/gwmpropertystatisticstab.h \
    gwmmaptoolidentifyfeature.h \
    gwmpropertypanel.h \
    gwmsymbolwindow.h \
    gwmtoolbar.h \
    mainwidget.h \
    mainwindow.h \
    prefix.h
    qgsattributetableviewextend.h

FORMS += \
    gwmfeaturepanel.ui \
    PropertyPanelTabs/gwmpropertydefaulttab.ui \
    PropertyPanelTabs/gwmpropertystatisticstab.ui \
    gwmpropertypanel.ui \
    gwmtoolbar.ui \
    mainwidget.ui \
    mainwindow.ui

## QGIS
INCLUDEPATH += "$(OSGEO_HOME)\include"
INCLUDEPATH += "$(OSGEO_HOME)\apps\qgis-dev\include"
LIBS += -L"$(OSGEO_HOME)\apps\qgis-dev\lib" -lqgis_core -lqgis_gui
GDAL_DATA = ".\share\gdal"
## QGIS END

TRANSLATIONS += \
    GWmodelDesktop_zh_CN.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
