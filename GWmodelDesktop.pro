QT       += core gui xml svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
CONFIG += qwt
CONFIG += resources_big
CONFIG += debug_and_release

QMAKE_CXXFLAGS += /openmp
QMAKE_CXXFLAGS_RELEASE += /Zi
QMAKE_CXXFLAGS_RELEASE += /Od
QMAKE_LFLAGS_RELEASE += /DEBUG


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += M_PI=3.14159265358979323846
DEFINES += M_PI_2=1.57079632679489661923
DEFINES += interface=struct

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    DelimitedText/qgsdelimitedtextfeatureiterator.cpp \
    DelimitedText/qgsdelimitedtextfile.cpp \
    DelimitedText/qgsdelimitedtextprovider.cpp \
    GWmodel/GWmodel.cpp \
    GWmodel/gwmbinomialmodel.cpp \
    GWmodel/gwmgeneralizedlinearmodel.cpp \
    GWmodel/gwmlinearmodel.cpp \
    GWmodel/gwmpoissonmodel.cpp \
    Model/gwmlayerattributeitem.cpp \
    Model/gwmlayerattributeitemmodel.cpp \
    Model/gwmlayerggwritem.cpp \
    Model/gwmlayergroupitem.cpp \
    Model/gwmlayergwritem.cpp \
    Model/gwmlayeritem.cpp \
    Model/gwmlayeritemmodel.cpp \
    Model/gwmlayermultiscalegwritem.cpp \
    Model/gwmlayeroriginitem.cpp \
    Model/gwmlayerscalablegwritem.cpp \
    Model/gwmlayersymbolitem.cpp \
    Model/gwmlayervectoritem.cpp \
    Model/gwmvariableitemmodel.cpp \
    PropertyPanelTabs/gwmpropertyggwrtab.cpp \
    Model/gwmparameterspecifiedoptionsmodel.cpp \
    Model/gwmpropertymultiscaleparameterspecifieditemmodel.cpp \
    PropertyPanelTabs/gwmpropertygwrtab.cpp \
    PropertyPanelTabs/gwmpropertymultiscalegwrtab.cpp \
    PropertyPanelTabs/gwmpropertyscalablegwrtab.cpp \
    SpatialWeight/gwmbandwidthweight.cpp \
    SpatialWeight/gwmcrsdistance.cpp \
    SpatialWeight/gwmdmatdistance.cpp \
    SpatialWeight/gwmminkwoskidistance.cpp \
    SpatialWeight/gwmspatialweight.cpp \
    TaskThread/gwmbandwidthselecttaskthread.cpp \
    TaskThread/gwmbandwidthsizeselector.cpp \
    TaskThread/gwmcsvtodatthread.cpp \
    TaskThread/gwmgeographicalweightedregressionalgorithm.cpp \
    TaskThread/gwmggwrbandwidthselectionthread.cpp \
    TaskThread/gwmggwrtaskthread.cpp \
    TaskThread/gwmgwpcataskthread.cpp \
    TaskThread/gwmgwrmodelselectionthread.cpp \
    TaskThread/gwmgwrtaskthread.cpp \
    TaskThread/gwmgwsstaskthread.cpp \
    TaskThread/gwmlcrgwrtaskthread.cpp \
    TaskThread/gwmmultiscalegwrtaskthread.cpp \
    TaskThread/gwmrobustgwrtaskthread.cpp \
    TaskThread/gwmsavelayerthread.cpp \
#    attributetable/qgsattributetabledelegate.cpp \
#    attributetable/qgsattributetablefiltermodel.cpp \
#    attributetable/qgsattributetablemodel.cpp \
#    attributetable/qgsattributetableview.cpp \
#    attributetable/qgsdualview.cpp \
    TaskThread/gwmscalablegwrtaskthread.cpp \
    TaskThread/gwmspatialalgorithm.cpp \
    TaskThread/gwmspatialmonoscalealgorithm.cpp \
    attributetable/qgsaddattrdialog.cpp \
    attributetable/qgsfeaturefilterwidget.cpp \
#    attributetable/qgsfeaturelistmodel.cpp \
#    attributetable/qgsfeaturelistview.cpp \
#    attributetable/qgsfeaturelistviewdelegate.cpp \
#    attributetable/qgsfeatureselectionmodel.cpp \
#    attributetable/qgsfieldconditionalformatwidget.cpp \
#    attributetable/qgsgenericfeatureselectionmanager.cpp \
#    attributetable/qgsorganizetablecolumnsdialog.cpp \
#    attributetable/qgsvectorlayerselectionmanager.cpp \
    attributetable/qgsfieldcalculator.cpp \
    attributetable/qgsguivectorlayertools.cpp \
    gwmattributetabledialog.cpp \
    gwmcoordtranssettingdialog.cpp \
    TaskThread/gwmcoordtransthread.cpp \
    gwmcsvtodatdialog.cpp \
    gwmdelimitedtextfile.cpp \
    gwmfeaturepanel.cpp \
    PropertyPanelTabs/gwmpropertydefaulttab.cpp \
    PropertyPanelTabs/gwmpropertystatisticstab.cpp \
    gwmggwroptionsdialog.cpp \
    gwmgwroptionsdialog.cpp \
    gwmindepvarselectorwidget.cpp \
    gwmlcrgwroptionsdialog.cpp \
    gwmmaptoolidentifyfeature.cpp \
    gwmmultiscalegwroptionsdialog.cpp \
    gwmopenxyeventlayerdialog.cpp \
    gwmplot.cpp \
    gwmprogressdialog.cpp \
    gwmpropertypanel.cpp \
    TaskThread/gwmtaskthread.cpp \
    gwmrobustgwroptionsdialog.cpp \
    gwmsaveascsvdialog.cpp \
    attributetable/qgsaddattrdialog.cpp \
#    attributetable/qgsclipboard.cpp \
    attributetable/qgsdelattrdialog.cpp \
    attributetable/qgsfeatureaction.cpp \
    attributetable/qgsfieldcalculator.cpp \
    gwmscalablegwroptionsdialog.cpp \
    symbolwindow/gwmsymbolwindow.cpp \
    gwmtoolbar.cpp \
    main.cpp \
    mainwidget.cpp \
    mainwindow.cpp

HEADERS += \
    DelimitedText/qgsdelimitedtextfeatureiterator.h \
    DelimitedText/qgsdelimitedtextfile.h \
    DelimitedText/qgsdelimitedtextprovider.h \
    GWmodel/GWmodel.h \
    GWmodel/gwmbinomialmodel.h \
    GWmodel/gwmgeneralizedlinearmodel.h \
    GWmodel/gwmlinearmodel.h \
    GWmodel/gwmpoissonmodel.h \
    Model/gwmlayerattributeitem.h \
    Model/gwmlayerattributeitemmodel.h \
    Model/gwmlayerggwritem.h \
    Model/gwmlayergroupitem.h \
    Model/gwmlayergwritem.h \
    Model/gwmlayeritem.h \
    Model/gwmlayeritemmodel.h \
    Model/gwmlayermultiscalegwritem.h \
    Model/gwmlayeroriginitem.h \
    Model/gwmlayerscalablegwritem.h \
    Model/gwmlayersymbolitem.h \
    Model/gwmlayervectoritem.h \
    Model/gwmvariableitemmodel.h \
    PropertyPanelTabs/gwmpropertyggwrtab.h \
    Model/gwmparameterspecifiedoptionsmodel.h \
    Model/gwmpropertymultiscaleparameterspecifieditemmodel.h \
    PropertyPanelTabs/gwmpropertygwrtab.h \
    PropertyPanelTabs/gwmpropertymultiscalegwrtab.h \
    PropertyPanelTabs/gwmpropertyscalablegwrtab.h \
    PropertyPanelTabs/utils.h \
    SpatialWeight/gwmbandwidthweight.h \
    SpatialWeight/gwmcrsdistance.h \
    SpatialWeight/gwmdistance.h \
    SpatialWeight/gwmdmatdistance.h \
    SpatialWeight/gwmminkwoskidistance.h \
    SpatialWeight/gwmspatialweight.h \
    TaskThread/gwmbandwidthselecttaskthread.h \
    TaskThread/gwmbandwidthsizeselector.h \
    TaskThread/gwmgeographicalweightedregressionalgorithm.h \
    TaskThread/gwmggwrbandwidthselectionthread.h \
    TaskThread/gwmggwrtaskthread.h \
    TaskThread/gwmgwpcataskthread.h \
    TaskThread/gwmgwrmodelselectionthread.h \
    TaskThread/gwmgwrtaskthread.h \
    TaskThread/gwmgwsstaskthread.h \
    TaskThread/gwmlcrgwrtaskthread.h \
    TaskThread/gwmmultiscalegwrtaskthread.h \
    TaskThread/gwmrobustgwrtaskthread.h \
    TaskThread/gwmrobustgwrtaskthread.h \
    TaskThread/gwmsavelayerthread.h \
#    attributetable/qgsattributetabledelegate.h \
#    attributetable/qgsattributetablefiltermodel.h \
#    attributetable/qgsattributetablemodel.h \
#    attributetable/qgsattributetableview.h \
#    attributetable/qgsdualview.h \
#    attributetable/qgsfeaturelistmodel.h \
#    attributetable/qgsfeaturelistview.h \
#    attributetable/qgsfeaturelistviewdelegate.h \
#    attributetable/qgsfeaturemodel.h \
#    attributetable/qgsfeatureselectionmodel.h \
#    attributetable/qgsfieldconditionalformatwidget.h \
#    attributetable/qgsgenericfeatureselectionmanager.h \
#    attributetable/qgsifeatureselectionmanager.h \
#    attributetable/qgsorganizetablecolumnsdialog.h \
#    attributetable/qgsvectorlayerselectionmanager.h \
    TaskThread/gwmscalablegwrtaskthread.h \
    TaskThread/gwmspatialalgorithm.h \
    TaskThread/gwmspatialmonoscalealgorithm.h \
    TaskThread/imonovariableanalysis.h \
    TaskThread/imultivariableanalysis.h \
    TaskThread/iparallelable.h \
    TaskThread/iregressionanalysis.h \
    attributetable/gwmfeaturefilterwidget_p.h \
    attributetable/qgsaddattrdialog.h \
    attributetable/qgsfieldcalculator.h \
    attributetable/qgsguivectorlayertools.h \
    gwmattributetabledialog.h \
    gwmcoordtranssettingdialog.h \
    TaskThread/gwmcoordtransthread.h \
    gwmcsvtodatdialog.h \
    gwmdelimitedtextfile.h \
    gwmenumvaluenamemapper.h \
    gwmfeaturepanel.h \
    PropertyPanelTabs/gwmpropertydefaulttab.h \
    PropertyPanelTabs/gwmpropertystatisticstab.h \
    gwmggwroptionsdialog.h \
    gwmgwroptionsdialog.h \
    gwmindepvarselectorwidget.h \
    gwmlcrgwroptionsdialog.h \
    gwmmaptoolidentifyfeature.h \
    gwmmultiscalegwroptionsdialog.h \
    gwmopenxyeventlayerdialog.h \
    gwmplot.h \
    gwmprogressdialog.h \
    gwmpropertypanel.h \
    TaskThread/gwmtaskthread.h \
    gwmrobustgwroptionsdialog.h \
    gwmsaveascsvdialog.h \
    attributetable/qgsaddattrdialog.h \
#    attributetable/qgsclipboard.h \
    attributetable/qgsdelattrdialog.h \
    attributetable/qgsfeatureaction.h \
    attributetable/qgsfieldcalculator.h \
    gwmscalablegwroptionsdialog.h \
    SpatialWeight/gwmweight.h \
    symbolwindow/gwmsymbolwindow.h \
    gwmtoolbar.h \
    mainwidget.h \
    mainwindow.h \
    prefix.h
    qgsattributetableviewextend.h

FORMS += \
    PropertyPanelTabs/gwmpropertyggwrtab.ui \
    PropertyPanelTabs/gwmpropertygwrtab.ui \
    PropertyPanelTabs/gwmpropertymultiscalegwrtab.ui \
    PropertyPanelTabs/gwmpropertyscalablegwrtab.ui \
    gwmcoordtranssettingdialog.ui \
    gwmcsvtodatdialog.ui \
    gwmfeaturepanel.ui \
    PropertyPanelTabs/gwmpropertydefaulttab.ui \
    PropertyPanelTabs/gwmpropertystatisticstab.ui \
    gwmggwroptionsdialog.ui \
    gwmgwroptionsdialog.ui \
    gwmindepvarselectorwidget.ui \
    gwmmultiscalegwroptionsdialog.ui \
    gwmlcrgwroptionsdialog.ui \
    gwmopenxyeventlayerdialog.ui \
    gwmprogressdialog.ui \
    gwmpropertypanel.ui \
    gwmrobustgwroptionsdialog.ui \
    gwmsaveascsvdialog.ui \
    gwmscalablegwroptionsdialog.ui \
    gwmtoolbar.ui \
    mainwidget.ui \
    mainwindow.ui \
    qgsaddattrdialogbase.ui \
    qgsattributetabledialog.ui \
    qgscoordinateoperationwidgetbase.ui \
    qgsdatumtransformdialogbase.ui \
    qgsdelattrdialogbase.ui \
    qgsdualviewbase.ui \
    qgseditconditionalformatrulewidget.ui \
    qgsfieldcalculatorbase.ui \
    qgsfieldconditionalformatwidget.ui \
    qgsorganizetablecolumnsdialog.ui \
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
CONFIG(debug, debug|release) {
    LIBS += -L"$(QT_HOME)/lib" -lqwtd
} else {
    LIBS += -L"$(QT_HOME)/lib" -lqwt
}
INCLUDEPATH += "$(QT_HOME)/include/qwt"
##Qwt END

## QGIS
INCLUDEPATH += "$(OSGEO_HOME)/include"
CONFIG(debug, debug|release) {
    INCLUDEPATH += "$(OSGEO_HOME)/apps/qgis-debug/include"
    LIBS += -L"$(OSGEO_HOME)/apps/qgis-debug/lib" -lqgis_core -lqgis_gui
} else {
    INCLUDEPATH += "$(OSGEO_HOME)/apps/qgis-rel-dev/include"
    LIBS += -L"$(OSGEO_HOME)/apps/qgis-rel-dev/lib" -lqgis_core -lqgis_gui
}
LIBS += -L"$(OSGEO_HOME)/lib" -lgdal_i
GDAL_DATA = ".\share\gdal"
## QGIS END

## Armadillo
DEFINES += ARMA_USE_LAPACK
DEFINES += ARMA_USE_BLAS
DEFINES += ARMA_DONT_USE_WRAPPER
INCLUDEPATH += "$(QT_HOME)/include/armadillo"
CONFIG(debug, debug|release) {
    LIBS += -L"$(QT_HOME)/lib" -larmadillod -lopenblas
} else {
    LIBS += -L"$(QT_HOME)/lib" -larmadillo -lopenblas
}
## Armadillo END

## GSL
INCLUDEPATH += "$(QT_HOME)/include/gsl"
LIBS += -L"$(QT_HOME)/lib" -lgsl -lgslcblas
## GSL END

TRANSLATIONS += \
    GWmodelDesktop_zh_CN.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    images/images.qrc \
    res.qrc
