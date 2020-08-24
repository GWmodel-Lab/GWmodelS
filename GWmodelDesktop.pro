QT       += core gui xml printsupport svg network

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
    Layout/gwmlayoutbatchconfigurationdelegate.cpp \
    Layout/gwmlayoutbatchconfigurationmodel.cpp \
    Layout/gwmlayoutbatchdialog.cpp \
    Layout/gwmlayoutbatchfieldlistmodel.cpp \
    Layout/gwmlayoutbatchlayerlistmodel.cpp \
    Layout/gwmlayoutbatchunifylayerpopupdialog.cpp \
    Layout/gwmlayoutdesigner.cpp \
    Layout/qgslayoutmanagerdialog.cpp \
    Model/gwmlayerattributeitem.cpp \
    Model/gwmlayerattributeitemmodel.cpp \
    Model/gwmlayerbasicgwritem.cpp \
    Model/gwmlayercollinearitygwritem.cpp \
    Model/gwmlayerggwritem.cpp \
    Model/gwmlayergroupitem.cpp \
    Model/gwmlayergtwritem.cpp \
    Model/gwmlayergwpcaitem.cpp \
    Model/gwmlayergwritem.cpp \
    Model/gwmlayergwssitem.cpp \
    Model/gwmlayeritem.cpp \
    Model/gwmlayeritemmodel.cpp \
    Model/gwmlayermultiscalegwritem.cpp \
    Model/gwmlayeroriginitem.cpp \
    Model/gwmlayerscalablegwritem.cpp \
    Model/gwmlayersymbolitem.cpp \
    Model/gwmlayervectoritem.cpp \
    Model/gwmvariableitemmodel.cpp \
    PropertyPanelTabs/gwmpropertycollinearitygwrtab.cpp \
    PropertyPanelTabs/gwmpropertyggwrtab.cpp \
    Model/gwmparameterspecifiedoptionsmodel.cpp \
    Model/gwmpropertymultiscaleparameterspecifieditemmodel.cpp \
    PropertyPanelTabs/gwmpropertygtwrtab.cpp \
    PropertyPanelTabs/gwmpropertygwpcatab.cpp \
    PropertyPanelTabs/gwmpropertygwrtab.cpp \
    PropertyPanelTabs/gwmpropertygwsstab.cpp \
    PropertyPanelTabs/gwmpropertymultiscalegwrtab.cpp \
    PropertyPanelTabs/gwmpropertyscalablegwrtab.cpp \
    SpatialWeight/gwmbandwidthweight.cpp \
    SpatialWeight/gwmcrsdistance.cpp \
    SpatialWeight/gwmdistance.cpp \
    SpatialWeight/gwmdmatdistance.cpp \
    SpatialWeight/gwmminkwoskidistance.cpp \
    SpatialWeight/gwmspatialtemporalweight.cpp \
    SpatialWeight/gwmspatialweight.cpp \
    SpatialWeight/gwmweight.cpp \
    TaskThread/gwmbandwidthselecttaskthread.cpp \
    TaskThread/gwmbandwidthsizeselector.cpp \
    TaskThread/gwmbasicgwralgorithm.cpp \
    TaskThread/gwmcsvtodatthread.cpp \
    TaskThread/gwmgeneralizedgwralgorithm.cpp \
    TaskThread/gwmgeographicalweightedregressionalgorithm.cpp \
#    TaskThread/gwmggwrbandwidthselectionthread.cpp \
#    TaskThread/gwmggwrtaskthread.cpp \
#    TaskThread/gwmgwpcataskthread.cpp \
    TaskThread/gwmgtwralgorithm.cpp \
    TaskThread/gwmgwpcataskthread.cpp \
    TaskThread/gwmggwrbandwidthsizeselector.cpp \
    TaskThread/gwmgwrmodelselectionthread.cpp \
    TaskThread/gwmgwrtaskthread.cpp \
    TaskThread/gwmgwsstaskthread.cpp \
    TaskThread/gwmindependentvariableselector.cpp \
    TaskThread/gwmlocalcollinearitygwralgorithm.cpp \
    TaskThread/gwmmultiscalegwralgorithm.cpp \
    TaskThread/gwmrobustgwralgorithm.cpp \
    TaskThread/gwmsavelayerthread.cpp \
#    attributetable/qgsattributetabledelegate.cpp \
#    attributetable/qgsattributetablefiltermodel.cpp \
#    attributetable/qgsattributetablemodel.cpp \
#    attributetable/qgsattributetableview.cpp \
#    attributetable/qgsdualview.cpp \
    TaskThread/gwmscalablegwralgorithm.cpp \
    TaskThread/gwmspatialalgorithm.cpp \
    TaskThread/gwmspatialmonoscalealgorithm.cpp \
    TaskThread/gwmspatialmultiscalealgorithm.cpp \
    TaskThread/gwmspatialtemporalmonoscale.cpp \
    Validity/qgsabstractvaliditycheck.cpp \
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
    gwmapp.cpp \
    gwmattributetabledialog.cpp \
    gwmcoordtranssettingdialog.cpp \
    TaskThread/gwmcoordtransthread.cpp \
    gwmcsvtodatdialog.cpp \
    gwmdelimitedtextfile.cpp \
    gwmfeaturepanel.cpp \
    PropertyPanelTabs/gwmpropertydefaulttab.cpp \
    PropertyPanelTabs/gwmpropertystatisticstab.cpp \
    gwmggwroptionsdialog.cpp \
    gwmgtwroptionsdialog.cpp \
    gwmgwpcaoptionsdialog.cpp \
    gwmgwroptionsdialog.cpp \
    gwmgwssoptionsdialog.cpp \
    gwmindepvarselectorwidget.cpp \
    gwmlcrgwroptionsdialog.cpp \
    gwmmaptoolidentifyfeature.cpp \
    gwmmultiscalegwroptionsdialog.cpp \
    gwmopenxyeventlayerdialog.cpp \
    gwmplot.cpp \
    gwmprogressdialog.cpp \
    gwmproject.cpp \
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
    symbolwindow/gwmsymboleditordialog.cpp \
    symbolwindow/gwmsymbolwindow.cpp \
    gwmtoolbar.cpp \
    main.cpp

HEADERS += \
    DelimitedText/qgsdelimitedtextfeatureiterator.h \
    DelimitedText/qgsdelimitedtextfile.h \
    DelimitedText/qgsdelimitedtextprovider.h \
    GWmodel/GWmodel.h \
    GWmodel/gwmbinomialmodel.h \
    GWmodel/gwmgeneralizedlinearmodel.h \
    GWmodel/gwmlinearmodel.h \
    GWmodel/gwmpoissonmodel.h \
    Layout/gwmlayoutbatchconfigurationdelegate.h \
    Layout/gwmlayoutbatchconfigurationmodel.h \
    Layout/gwmlayoutbatchdialog.h \
    Layout/gwmlayoutbatchfieldlistmodel.h \
    Layout/gwmlayoutbatchlayerlistmodel.h \
    Layout/gwmlayoutbatchunifylayerpopupdialog.h \
    Layout/gwmlayoutdesigner.h \
    Layout/qgslayoutmanagerdialog.h \
    Model/gwmlayerattributeitem.h \
    Model/gwmlayerattributeitemmodel.h \
    Model/gwmlayerbasicgwritem.h \
    Model/gwmlayercollinearitygwritem.h \
    Model/gwmlayerggwritem.h \
    Model/gwmlayergroupitem.h \
    Model/gwmlayergtwritem.h \
    Model/gwmlayergwpcaitem.h \
    Model/gwmlayergwritem.h \
    Model/gwmlayergwssitem.h \
    Model/gwmlayeritem.h \
    Model/gwmlayeritemmodel.h \
    Model/gwmlayermultiscalegwritem.h \
    Model/gwmlayeroriginitem.h \
    Model/gwmlayerscalablegwritem.h \
    Model/gwmlayersymbolitem.h \
    Model/gwmlayervectoritem.h \
    Model/gwmvariableitemmodel.h \
    PropertyPanelTabs/gwmpropertycollinearitygwrtab.h \
    PropertyPanelTabs/gwmpropertyggwrtab.h \
    Model/gwmparameterspecifiedoptionsmodel.h \
    Model/gwmpropertymultiscaleparameterspecifieditemmodel.h \
    PropertyPanelTabs/gwmpropertygtwrtab.h \
    PropertyPanelTabs/gwmpropertygwpcatab.h \
    PropertyPanelTabs/gwmpropertygwrtab.h \
    PropertyPanelTabs/gwmpropertygwsstab.h \
    PropertyPanelTabs/gwmpropertymultiscalegwrtab.h \
    PropertyPanelTabs/gwmpropertyscalablegwrtab.h \
    PropertyPanelTabs/utils.h \
    SpatialWeight/gwmbandwidthweight.h \
    SpatialWeight/gwmcrsdistance.h \
    SpatialWeight/gwmdistance.h \
    SpatialWeight/gwmdmatdistance.h \
    SpatialWeight/gwmminkwoskidistance.h \
    SpatialWeight/gwmspatialtemporalweight.h \
    SpatialWeight/gwmspatialweight.h \
    TaskThread/gwmbandwidthselecttaskthread.h \
    TaskThread/gwmbandwidthsizeselector.h \
    TaskThread/gwmbasicgwralgorithm.h \
    TaskThread/gwmgeneralizedgwralgorithm.h \
    TaskThread/gwmgeographicalweightedregressionalgorithm.h \
#    TaskThread/gwmggwrbandwidthselectionthread.h \
#    TaskThread/gwmggwrtaskthread.h \
#    TaskThread/gwmgwpcataskthread.h \
    TaskThread/gwmgtwralgorithm.h \
    TaskThread/gwmgwpcataskthread.h \
    TaskThread/gwmggwrbandwidthsizeselector.h \
    TaskThread/gwmgwrmodelselectionthread.h \
    TaskThread/gwmgwrtaskthread.h \
    TaskThread/gwmgwsstaskthread.h \
    TaskThread/gwmindependentvariableselector.h \
    TaskThread/gwmlocalcollinearitygwralgorithm.h \
    TaskThread/gwmmultiscalegwralgorithm.h \
    TaskThread/gwmrobustgwralgorithm.h \
    TaskThread/gwmrobustgwralgorithm.h \
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
    TaskThread/gwmscalablegwralgorithm.h \
    TaskThread/gwmspatialalgorithm.h \
    TaskThread/gwmspatialmonoscalealgorithm.h \
    TaskThread/gwmspatialmultiscalealgorithm.h \
    TaskThread/gwmspatialtemporalmonoscale.h \
    TaskThread/imonovariableanalysis.h \
    TaskThread/imultivariableanalysis.h \
    TaskThread/iparallelable.h \
    TaskThread/iregressionanalysis.h \
    Validity/qgsabstractvaliditycheck.h \
    attributetable/gwmfeaturefilterwidget_p.h \
    attributetable/qgsaddattrdialog.h \
    attributetable/qgsfieldcalculator.h \
    attributetable/qgsguivectorlayertools.h \
    gwmapp.h \
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
    gwmgtwroptionsdialog.h \
    gwmgwpcaoptionsdialog.h \
    gwmgwroptionsdialog.h \
    gwmgwssoptionsdialog.h \
    gwmindepvarselectorwidget.h \
    gwmlcrgwroptionsdialog.h \
    gwmmaptoolidentifyfeature.h \
    gwmmultiscalegwroptionsdialog.h \
    gwmopenxyeventlayerdialog.h \
    gwmplot.h \
    gwmprogressdialog.h \
    gwmproject.h \
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
    symbolwindow/gwmsymboleditordialog.h \
    symbolwindow/gwmsymbolwindow.h \
    gwmtoolbar.h \
    prefix.h
    qgsattributetableviewextend.h

FORMS += \
    Layout/gwmlayoutbatchdialog.ui \
    Layout/gwmlayoutbatchunifylayerpopupdialog.ui \
    Layout/gwmlayoutdesigner.ui \
    Layout/qgslayoutatlaswidgetbase.ui \
    Layout/qgslayoutmanagerbase.ui \
    Layout/qgssvgexportoptions.ui \
    PropertyPanelTabs/gwmpropertycollinearitygwrtab.ui \
    PropertyPanelTabs/gwmpropertyggwrtab.ui \
    PropertyPanelTabs/gwmpropertygtwrtab.ui \
    PropertyPanelTabs/gwmpropertygwpcatab.ui \
    PropertyPanelTabs/gwmpropertygwrtab.ui \
    PropertyPanelTabs/gwmpropertygwsstab.ui \
    PropertyPanelTabs/gwmpropertymultiscalegwrtab.ui \
    PropertyPanelTabs/gwmpropertyscalablegwrtab.ui \
    gwmapp.ui \
    gwmcoordtranssettingdialog.ui \
    gwmcsvtodatdialog.ui \
    gwmfeaturepanel.ui \
    PropertyPanelTabs/gwmpropertydefaulttab.ui \
    PropertyPanelTabs/gwmpropertystatisticstab.ui \
    gwmggwroptionsdialog.ui \
    gwmgtwroptionsdialog.ui \
    gwmgwpcaoptionsdialog.ui \
    gwmgwroptionsdialog.ui \
    gwmgwssoptionsdialog.ui \
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
    qgsaddattrdialogbase.ui \
    qgsattributetabledialog.ui \
    qgscoordinateoperationwidgetbase.ui \
    qgsdatumtransformdialogbase.ui \
    qgsdelattrdialogbase.ui \
    qgsdualviewbase.ui \
    qgseditconditionalformatrulewidget.ui \
    qgsexpressionpreviewbase.ui \
    qgsfieldcalculatorbase.ui \
    qgsfieldconditionalformatwidget.ui \
    qgsorganizetablecolumnsdialog.ui \
    qgsvaliditycheckresultsbase.ui \
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

## GWmodelCUDA
INCLUDEPATH += "$(QT_HOME)/include/GWmodelCUDA"
LIBS += -L"$(QT_HOME)/lib" -lGWmodelCUDA64 -lCUDAInspector


## GWmodelCUDA END

TRANSLATIONS += \
    GWmodelDesktop_zh_CN.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    images/images.qrc \
    res.qrc

