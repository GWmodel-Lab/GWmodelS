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
    Model/gwmlayergroupitem.cpp \
    Model/gwmlayergwritem.cpp \
    Model/gwmlayeritem.cpp \
    Model/gwmlayeritemmodel.cpp \
    Model/gwmlayeroriginitem.cpp \
    Model/gwmlayersymbolitem.cpp \
    Model/gwmlayervectoritem.cpp \
    TaskThread/gwmsavelayerthread.cpp \
    gwmattributetableview.cpp \
    gwmcoordtranssettingdialog.cpp \
    TaskThread/gwmcoordtransthread.cpp \
    gwmdelimitedtextfile.cpp \
    gwmfeaturepanel.cpp \
    PropertyPanelTabs/gwmpropertydefaulttab.cpp \
    PropertyPanelTabs/gwmpropertystatisticstab.cpp \
    gwmgwroptionsdialog.cpp \
    gwmmaptoolidentifyfeature.cpp \
    gwmopenxyeventlayerdialog.cpp \
    gwmprogressdialog.cpp \
    gwmpropertypanel.cpp \
    TaskThread/gwmtaskthread.cpp \
    gwmsaveascsvdialog.cpp \
    symbolwindow/gwmsymbolwindow.cpp \
    gwmtoolbar.cpp \
    main.cpp \
    mainwidget.cpp \
    mainwindow.cpp

HEADERS += \
    DelimitedText/qgsdelimitedtextfeatureiterator.h \
    DelimitedText/qgsdelimitedtextfile.h \
    DelimitedText/qgsdelimitedtextprovider.h \
    Model/gwmlayergroupitem.h \
    Model/gwmlayergwritem.h \
    Model/gwmlayeritem.h \
    Model/gwmlayeritemmodel.h \
    Model/gwmlayeroriginitem.h \
    Model/gwmlayersymbolitem.h \
    Model/gwmlayervectoritem.h \
    PropertyPanelTabs/utils.h \
    TaskThread/gwmsavelayerthread.h \
    gwmattributetableview.h \
    gwmcoordtranssettingdialog.h \
    TaskThread/gwmcoordtransthread.h \
    gwmdelimitedtextfile.h \
    gwmfeaturepanel.h \
    PropertyPanelTabs/gwmpropertydefaulttab.h \
    PropertyPanelTabs/gwmpropertystatisticstab.h \
    gwmgwroptionsdialog.h \
    gwmmaptoolidentifyfeature.h \
    gwmopenxyeventlayerdialog.h \
    gwmprogressdialog.h \
    gwmpropertypanel.h \
    TaskThread/gwmtaskthread.h \
    gwmsaveascsvdialog.h \
    symbolwindow/gwmsymbolwindow.h \
    gwmtoolbar.h \
    mainwidget.h \
    mainwindow.h \
    prefix.h
    qgsattributetableviewextend.h

FORMS += \
    gwmcoordtranssettingdialog.ui \
    gwmfeaturepanel.ui \
    PropertyPanelTabs/gwmpropertydefaulttab.ui \
    PropertyPanelTabs/gwmpropertystatisticstab.ui \
    gwmgwroptionsdialog.ui \
    gwmopenxyeventlayerdialog.ui \
    gwmprogressdialog.ui \
    gwmpropertypanel.ui \
    gwmsaveascsvdialog.ui \
    gwmtoolbar.ui \
    mainwidget.ui \
    mainwindow.ui \
    qgscoordinateoperationwidgetbase.ui \
    qgsdatumtransformdialogbase.ui \
    symbolwindow/qgscategorizedsymbolrendererwidget.ui \
    symbolwindow/qgsgraduatedsymbolrendererwidget.ui \
    symbolwindow/qgsheatmaprendererwidgetbase.ui \
    symbolwindow/qgshistogramwidgetbase.ui \
    symbolwindow/qgsinvertedpolygonrendererwidgetbase.ui \
    symbolwindow/qgsmapunitscalewidgetbase.ui \
    symbolwindow/qgspointclusterrendererwidgetbase.ui \
    symbolwindow/qgspointdisplacementrendererwidgetbase.ui \
    symbolwindow/qgsrendererrulepropsdialogbase.ui \
    symbolwindow/qgsrulebasedrendererwidget.ui \
    symbolwindow/qgsstyleitemslistwidgetbase.ui \
    symbolwindow/qgssymbolselectordialogbase.ui \
    symbolwindow/qgsunitselectionwidget.ui \
    symbolwindow/widget_set_dd_value.ui
##Qwt
DEFINES += QT_DLL QWT_DLL
LIBS += -L"$(QT_HOME)/lib" -lqwt
INCLUDEPATH += "$(QT_HOME)/include/qwt"
##Qwt END

## QGIS
INCLUDEPATH += "$(OSGEO_HOME)/include"
INCLUDEPATH += "$(OSGEO_HOME)/apps/qgis-dev/include"
LIBS += -L"$(OSGEO_HOME)/apps/qgis-dev/lib" -lqgis_core -lqgis_gui
LIBS += -L"$(OSGEO_HOME)/lib" -lgdal_i
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
