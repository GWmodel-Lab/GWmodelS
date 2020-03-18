QT       += core gui xml svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
CONFIG += qwt

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
    DelimitedText/qgsdelimitedtextfeatureiterator.cpp \
    DelimitedText/qgsdelimitedtextfile.cpp \
    DelimitedText/qgsdelimitedtextprovider.cpp \
    gwmattributetableview.cpp \
    gwmdelimitedtextfile.cpp \
    gwmfeaturepanel.cpp \
    PropertyPanelTabs/gwmpropertydefaulttab.cpp \
    PropertyPanelTabs/gwmpropertystatisticstab.cpp \
    gwmmaptoolidentifyfeature.cpp \
    gwmopenxyeventlayerdialog.cpp \
    gwmpropertypanel.cpp \
    gwmsymbolwindow.cpp \
    gwmtoolbar.cpp \
    main.cpp \
    mainwidget.cpp \
    mainwindow.cpp

HEADERS += \
    DelimitedText/qgsdelimitedtextfeatureiterator.h \
    DelimitedText/qgsdelimitedtextfile.h \
    DelimitedText/qgsdelimitedtextprovider.h \
    gwmattributetableview.h \
    gwmdelimitedtextfile.h \
    gwmfeaturepanel.h \
    PropertyPanelTabs/gwmpropertydefaulttab.h \
    PropertyPanelTabs/gwmpropertystatisticstab.h \
    gwmmaptoolidentifyfeature.h \
    gwmopenxyeventlayerdialog.h \
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
    gwmopenxyeventlayerdialog.ui \
    gwmpropertypanel.ui \
    gwmtoolbar.ui \
    mainwidget.ui \
    mainwindow.ui \
    qgscategorizedsymbolrendererwidget.ui \
    qgsgraduatedsymbolrendererwidget.ui \
    qgsheatmaprendererwidgetbase.ui \
    qgshistogramwidgetbase.ui \
    qgsinvertedpolygonrendererwidgetbase.ui \
    qgsmapunitscalewidgetbase.ui \
    qgspointclusterrendererwidgetbase.ui \
    qgspointdisplacementrendererwidgetbase.ui \
    qgsrendererrulepropsdialogbase.ui \
    qgsrulebasedrendererwidget.ui \
    qgsstyleitemslistwidgetbase.ui \
    qgssymbolselectordialogbase.ui \
    qgsunitselectionwidget.ui \
    widget_set_dd_value.ui

##Qwt
DEFINES += QT_DLL QWT_DLL
LIBS += -L"C:\Qt\5.12.7\msvc2017_64\lib" -lqwtd -lqwt
INCLUDEPATH += "C:\Qt\5.12.7\msvc2017_64\include\qwt"
##Qwt END

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
    images/images.qrc \
    res.qrc
