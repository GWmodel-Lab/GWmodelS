cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)
project(GWmodelDesktop LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
# set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Gui Xml PrintSupport Svg Network REQUIRED)

find_package(GDAL REQUIRED)
if(NOT GDAL_FOUND)
    message(FATAL_ERROR "GDAL Not Found!")
endif(NOT GDAL_FOUND)
include_directories(${GDAL_INCLUDE_DIR})

find_package(GEOS REQUIRED)
if(NOT GEOS_FOUND)
    message(FATAL_ERROR "GEOS Not Found!")
endif(NOT GEOS_FOUND)
include_directories(${GEOS_INCLUDE_DIR})

find_package(SQLite3 REQUIRED)
if(NOT SQLITE3_FOUND)
    message(FATAL_ERROR "SQLite3 Not Found!")
endif(NOT SQLITE3_FOUND)
include_directories(${SQLITE3_INCLUDE_DIR})

find_package(QScintilla REQUIRED)
if(NOT QSCINTILLA_FOUND)
    message(FATAL_ERROR "QScintilla Not Found!")
endif(NOT QSCINTILLA_FOUND)
include_directories(${QSCINTILLA_INCLUDE_DIR})

find_package(QGIS REQUIRED)
if(NOT QGIS_FOUND)
    message(FATAL_ERROR "QGIS Not Found!")
endif(NOT QGIS_FOUND)
include_directories(${QGIS_INCLUDE_DIR} ${QGIS_UI_INCLUDE_DIR})

find_package(Qwt REQUIRED)
if(NOT QWT_FOUND)
    message(FATAL_ERROR "Qwt Not Found!")
endif(NOT QWT_FOUND)
include_directories(${QWT_INCLUDE_DIR})

find_package(OpenMP REQUIRED)
if(NOT OpenMP_FOUND)
    message(FATAL_ERROR "OpenMP Not Found!")
endif(NOT OpenMP_FOUND)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS"${CMAKE_EXE_LINKER_FLAGS}${OpenMP_EXE_LINKER_FLAGS}")
include_directories(${OpenMP_C_INCLUDE_DIRS} ${OpenMP_CXX_INCLUDE_DIRS})

find_package(Armadillo REQUIRED)
if(NOT ARMADILLO_FOUND)
    message(FATAL_ERROR "Armadillo Not Found!")
else(NOT ARMADILLO_FOUND)
    add_definitions(-DARMA_USE_LAPACK -DARMA_USE_BLAS -DARMA_DONT_USE_WRAPPER)
    include_directories(${ARMADILLO_INCLUDE_DIRS})
    
    find_package(BLAS)
    if(NOT BLAS_FOUND)
    message(FATAL_ERROR "BLAS Not Found!")
    endif(NOT BLAS_FOUND)
    
    find_package(LAPACK)
    if(NOT LAPACK_FOUND)
    message(FATAL_ERROR "LAPACK Not Found!")
    endif(NOT LAPACK_FOUND)
endif(NOT ARMADILLO_FOUND )

find_package(GSL REQUIRED)
if(NOT GSL_FOUND)
    message(FATAL_ERROR "GSL Not Found!")
endif(NOT GSL_FOUND)
include_directories(${GSL_INCLUDE_DIRS})


if(NOT APPLE)
    set (ENABLE_CUDA FALSE CACHE BOOL "Determines whether CUDA support should be built")
    if(ENABLE_CUDA)
        find_package(GWmodelCUDA REQUIRED)
        if(NOT GWMODELCUDA_FOUND)
            message(FATAL_ERROR "GWmodelCUDA Not Found!")
        endif(NOT GWMODELCUDA_FOUND)
        add_definitions(-D ENABLE_CUDA)
        include_directories(${GWMODELCUDA_INCLUDE_DIR})
    endif(ENABLE_CUDA)    
endif(NOT APPLE)


add_definitions(-D M_PI=3.14159265358979323846 -D M_PI_2=1.57079632679489661923)
if(NOT WIN32)
    add_definitions(-D DBL_MAX=__DBL_MAX__)
endif(NOT WIN32)

set(SOURCES 
    DelimitedText/qgsdelimitedtextfeatureiterator.cpp
    DelimitedText/qgsdelimitedtextfile.cpp
    DelimitedText/qgsdelimitedtextprovider.cpp
    GWmodel/GWmodel.cpp
    GWmodel/gwmbinomialmodel.cpp
    GWmodel/gwmgeneralizedlinearmodel.cpp
    GWmodel/gwmlinearmodel.cpp
    GWmodel/gwmpoissonmodel.cpp
    Layout/gwmlayoutbatchconfigurationdelegate.cpp
    Layout/gwmlayoutbatchconfigurationmodel.cpp
    Layout/gwmlayoutbatchdialog.cpp
    Layout/gwmlayoutbatchfieldlistmodel.cpp
    Layout/gwmlayoutbatchlayerlistmodel.cpp
    Layout/gwmlayoutbatchunifylayerpopupdialog.cpp
    Layout/gwmlayoutdesigner.cpp
    Layout/qgslayoutmanagerdialog.cpp
    Model/gwmlayerattributeitem.cpp
    Model/gwmlayerattributeitemmodel.cpp
    Model/gwmlayerbasicgwritem.cpp
    Model/gwmlayercollinearitygwritem.cpp
    Model/gwmlayerggwritem.cpp
    Model/gwmlayergroupitem.cpp
    Model/gwmlayergtwritem.cpp
    Model/gwmlayergwpcaitem.cpp
    Model/gwmlayergwssitem.cpp
    Model/gwmlayeritem.cpp
    Model/gwmlayeritemmodel.cpp
    Model/gwmlayermultiscalegwritem.cpp
    Model/gwmlayeroriginitem.cpp
    Model/gwmlayerscalablegwritem.cpp
    Model/gwmlayersymbolitem.cpp
    Model/gwmlayervectoritem.cpp
    Model/gwmvariableitemmodel.cpp
    PropertyPanelTabs/gwmpropertycollinearitygwrtab.cpp
    PropertyPanelTabs/gwmpropertyggwrtab.cpp
    Model/gwmparameterspecifiedoptionsmodel.cpp
    Model/gwmpropertymultiscaleparameterspecifieditemmodel.cpp
    PropertyPanelTabs/gwmpropertygtwrtab.cpp
    PropertyPanelTabs/gwmpropertygwpcatab.cpp
    PropertyPanelTabs/gwmpropertygwrtab.cpp
    PropertyPanelTabs/gwmpropertygwsstab.cpp
    PropertyPanelTabs/gwmpropertymultiscalegwrtab.cpp
    PropertyPanelTabs/gwmpropertyscalablegwrtab.cpp
    SpatialWeight/gwmbandwidthweight.cpp
    SpatialWeight/gwmcrsdistance.cpp
    SpatialWeight/gwmdistance.cpp
    SpatialWeight/gwmdmatdistance.cpp
    SpatialWeight/gwmminkwoskidistance.cpp
    SpatialWeight/gwmspatialtemporalweight.cpp
    SpatialWeight/gwmspatialweight.cpp
    SpatialWeight/gwmweight.cpp
    TaskThread/gwmbandwidthsizeselector.cpp
    TaskThread/gwmbasicgwralgorithm.cpp
    TaskThread/gwmcsvtodatthread.cpp
    TaskThread/gwmgeneralizedgwralgorithm.cpp
    TaskThread/gwmgeographicalweightedregressionalgorithm.cpp
    TaskThread/gwmgtwralgorithm.cpp
    TaskThread/gwmgwpcataskthread.cpp
    TaskThread/gwmggwrbandwidthsizeselector.cpp
    TaskThread/gwmgwsstaskthread.cpp
    TaskThread/gwmindependentvariableselector.cpp
    TaskThread/gwmlocalcollinearitygwralgorithm.cpp
    TaskThread/gwmmultiscalegwralgorithm.cpp
    TaskThread/gwmrobustgwralgorithm.cpp
    TaskThread/gwmsavelayerthread.cpp
    TaskThread/gwmscalablegwralgorithm.cpp
    TaskThread/gwmspatialalgorithm.cpp
    TaskThread/gwmspatialmonoscalealgorithm.cpp
    TaskThread/gwmspatialmultiscalealgorithm.cpp
    TaskThread/gwmspatialtemporalmonoscale.cpp
    Validity/qgsabstractvaliditycheck.cpp
    attributetable/qgsfeaturefilterwidget.cpp
    attributetable/qgsguivectorlayertools.cpp
    gwmapp.cpp
    gwmattributetabledialog.cpp
    gwmcoordtranssettingdialog.cpp
    TaskThread/gwmcoordtransthread.cpp
    gwmcsvtodatdialog.cpp
    gwmdelimitedtextfile.cpp
    gwmfeaturepanel.cpp
    PropertyPanelTabs/gwmpropertydefaulttab.cpp
    PropertyPanelTabs/gwmpropertystatisticstab.cpp
    gwmggwroptionsdialog.cpp
    gwmgtwroptionsdialog.cpp
    gwmgwpcaoptionsdialog.cpp
    gwmgwroptionsdialog.cpp
    gwmgwssoptionsdialog.cpp
    gwmindepvarselectorwidget.cpp
    gwmlcrgwroptionsdialog.cpp
    gwmmaptoolidentifyfeature.cpp
    gwmmultiscalegwroptionsdialog.cpp
    gwmopenxyeventlayerdialog.cpp
    gwmplot.cpp
    gwmprogressdialog.cpp
    gwmproject.cpp
    gwmpropertypanel.cpp
    TaskThread/gwmtaskthread.cpp
    gwmrobustgwroptionsdialog.cpp
    gwmsaveascsvdialog.cpp
    attributetable/qgsdelattrdialog.cpp
    attributetable/qgsfeatureaction.cpp
    gwmscalablegwroptionsdialog.cpp
    symbolwindow/gwmsymboleditordialog.cpp
    symbolwindow/gwmsymbolwindow.cpp
    gwmtoolbar.cpp
    main.cpp
)

set(HEADERS
    DelimitedText/qgsdelimitedtextfeatureiterator.h
    DelimitedText/qgsdelimitedtextfile.h
    DelimitedText/qgsdelimitedtextprovider.h
    GWmodel/GWmodel.h
    GWmodel/gwmbinomialmodel.h
    GWmodel/gwmgeneralizedlinearmodel.h
    GWmodel/gwmlinearmodel.h
    GWmodel/gwmpoissonmodel.h
    Layout/gwmlayoutbatchconfigurationdelegate.h
    Layout/gwmlayoutbatchconfigurationmodel.h
    Layout/gwmlayoutbatchdialog.h
    Layout/gwmlayoutbatchfieldlistmodel.h
    Layout/gwmlayoutbatchlayerlistmodel.h
    Layout/gwmlayoutbatchunifylayerpopupdialog.h
    Layout/gwmlayoutdesigner.h
    Layout/qgslayoutmanagerdialog.h
    Model/gwmlayerattributeitem.h
    Model/gwmlayerattributeitemmodel.h
    Model/gwmlayerbasicgwritem.h
    Model/gwmlayercollinearitygwritem.h
    Model/gwmlayerggwritem.h
    Model/gwmlayergroupitem.h
    Model/gwmlayergtwritem.h
    Model/gwmlayergwpcaitem.h
    Model/gwmlayergwssitem.h
    Model/gwmlayeritem.h
    Model/gwmlayeritemmodel.h
    Model/gwmlayermultiscalegwritem.h
    Model/gwmlayeroriginitem.h
    Model/gwmlayerscalablegwritem.h
    Model/gwmlayersymbolitem.h
    Model/gwmlayervectoritem.h
    Model/gwmvariableitemmodel.h
    PropertyPanelTabs/gwmpropertycollinearitygwrtab.h
    PropertyPanelTabs/gwmpropertyggwrtab.h
    Model/gwmparameterspecifiedoptionsmodel.h
    Model/gwmpropertymultiscaleparameterspecifieditemmodel.h
    PropertyPanelTabs/gwmpropertygtwrtab.h
    PropertyPanelTabs/gwmpropertygwpcatab.h
    PropertyPanelTabs/gwmpropertygwrtab.h
    PropertyPanelTabs/gwmpropertygwsstab.h
    PropertyPanelTabs/gwmpropertymultiscalegwrtab.h
    PropertyPanelTabs/gwmpropertyscalablegwrtab.h
    PropertyPanelTabs/utils.h
    SpatialWeight/gwmbandwidthweight.h
    SpatialWeight/gwmcrsdistance.h
    SpatialWeight/gwmdistance.h
    SpatialWeight/gwmdmatdistance.h
    SpatialWeight/gwmminkwoskidistance.h
    SpatialWeight/gwmspatialtemporalweight.h
    SpatialWeight/gwmspatialweight.h
    TaskThread/gwmbandwidthsizeselector.h
    TaskThread/gwmbasicgwralgorithm.h
    TaskThread/gwmgeneralizedgwralgorithm.h
    TaskThread/gwmgeographicalweightedregressionalgorithm.h
    TaskThread/gwmgtwralgorithm.h
    TaskThread/gwmgwpcataskthread.h
    TaskThread/gwmggwrbandwidthsizeselector.h
    TaskThread/gwmgwsstaskthread.h
    TaskThread/gwmindependentvariableselector.h
    TaskThread/gwmlocalcollinearitygwralgorithm.h
    TaskThread/gwmmultiscalegwralgorithm.h
    TaskThread/gwmrobustgwralgorithm.h
    TaskThread/gwmrobustgwralgorithm.h
    TaskThread/gwmsavelayerthread.h
    TaskThread/gwmscalablegwralgorithm.h
    TaskThread/gwmspatialalgorithm.h
    TaskThread/gwmspatialmonoscalealgorithm.h
    TaskThread/gwmspatialmultiscalealgorithm.h
    TaskThread/gwmspatialtemporalmonoscale.h
    TaskThread/imonovariableanalysis.h
    TaskThread/imultivariableanalysis.h
    TaskThread/iparallelable.h
    TaskThread/iregressionanalysis.h
    Validity/qgsabstractvaliditycheck.h
    attributetable/gwmfeaturefilterwidget_p.h
    attributetable/qgsguivectorlayertools.h
    gwmapp.h
    gwmattributetabledialog.h
    gwmcoordtranssettingdialog.h
    TaskThread/gwmcoordtransthread.h
    gwmcsvtodatdialog.h
    gwmdelimitedtextfile.h
    gwmenumvaluenamemapper.h
    gwmfeaturepanel.h
    PropertyPanelTabs/gwmpropertydefaulttab.h
    PropertyPanelTabs/gwmpropertystatisticstab.h
    gwmggwroptionsdialog.h
    gwmgtwroptionsdialog.h
    gwmgwpcaoptionsdialog.h
    gwmgwroptionsdialog.h
    gwmgwssoptionsdialog.h
    gwmindepvarselectorwidget.h
    gwmlcrgwroptionsdialog.h
    gwmmaptoolidentifyfeature.h
    gwmmultiscalegwroptionsdialog.h
    gwmopenxyeventlayerdialog.h
    gwmplot.h
    gwmprogressdialog.h
    gwmproject.h
    gwmpropertypanel.h
    TaskThread/gwmtaskthread.h
    gwmrobustgwroptionsdialog.h
    gwmsaveascsvdialog.h
    attributetable/qgsdelattrdialog.h
    attributetable/qgsfeatureaction.h
    gwmscalablegwroptionsdialog.h
    SpatialWeight/gwmweight.h
    symbolwindow/gwmsymboleditordialog.h
    symbolwindow/gwmsymbolwindow.h
    gwmtoolbar.h
    prefix.h
)

set(FORMS
    Layout/gwmlayoutbatchdialog.ui
    Layout/gwmlayoutbatchunifylayerpopupdialog.ui
    Layout/gwmlayoutdesigner.ui
    Layout/qgslayoutatlaswidgetbase.ui
    Layout/qgslayoutmanagerbase.ui
    Layout/qgssvgexportoptions.ui
    PropertyPanelTabs/gwmpropertycollinearitygwrtab.ui
    PropertyPanelTabs/gwmpropertyggwrtab.ui
    PropertyPanelTabs/gwmpropertygtwrtab.ui
    PropertyPanelTabs/gwmpropertygwpcatab.ui
    PropertyPanelTabs/gwmpropertygwrtab.ui
    PropertyPanelTabs/gwmpropertygwsstab.ui
    PropertyPanelTabs/gwmpropertymultiscalegwrtab.ui
    PropertyPanelTabs/gwmpropertyscalablegwrtab.ui
    gwmapp.ui
    gwmcoordtranssettingdialog.ui
    gwmcsvtodatdialog.ui
    gwmfeaturepanel.ui
    PropertyPanelTabs/gwmpropertydefaulttab.ui
    PropertyPanelTabs/gwmpropertystatisticstab.ui
    gwmggwroptionsdialog.ui
    gwmgtwroptionsdialog.ui
    gwmgwpcaoptionsdialog.ui
    gwmgwroptionsdialog.ui
    gwmgwssoptionsdialog.ui
    gwmindepvarselectorwidget.ui
    gwmmultiscalegwroptionsdialog.ui
    gwmlcrgwroptionsdialog.ui
    gwmopenxyeventlayerdialog.ui
    gwmprogressdialog.ui
    gwmpropertypanel.ui
    gwmrobustgwroptionsdialog.ui
    gwmsaveascsvdialog.ui
    gwmscalablegwroptionsdialog.ui
    gwmtoolbar.ui
    qgsaddattrdialogbase.ui
    qgsattributetabledialog.ui
    qgscoordinateoperationwidgetbase.ui
    qgsdatumtransformdialogbase.ui
    qgsdelattrdialogbase.ui
    qgsdualviewbase.ui
    qgseditconditionalformatrulewidget.ui
    qgsexpressionpreviewbase.ui
    qgsfieldcalculatorbase.ui
    qgsfieldconditionalformatwidget.ui
    qgsorganizetablecolumnsdialog.ui
    qgsvaliditycheckresultsbase.ui
    symbolwindow/qgsgraduatedsymbolrendererwidget.ui
    symbolwindow/qgscategorizedsymbolrendererwidget.ui
    symbolwindow/qgsheatmaprendererwidgetbase.ui
    symbolwindow/qgshistogramwidgetbase.ui
    symbolwindow/qgsinvertedpolygonrendererwidgetbase.ui
    symbolwindow/qgsmapunitscalewidgetbase.ui
    symbolwindow/qgspointclusterrendererwidgetbase.ui
    symbolwindow/qgspointdisplacementrendererwidgetbase.ui
    symbolwindow/qgsrendererrulepropsdialogbase.ui
    symbolwindow/qgsrulebasedrendererwidget.ui
    symbolwindow/qgsstyleitemslistwidgetbase.ui
    symbolwindow/qgssymbolselectordialogbase.ui
    symbolwindow/qgsunitselectionwidget.ui
    symbolwindow/widget_set_dd_value.ui
)

qt5_wrap_ui(FORMS_H ${FORMS})

if(WIN32)
    set(QT_USEQTMAIN TRUE)
    add_executable(GWmodelDesktop WIN32 ${SOURCES} ${HEADERS} ${FORMS_H})
else(WIN32)
    add_executable(GWmodelDesktop ${SOURCES} ${HEADERS} ${FORMS_H})
endif(WIN32)


target_link_libraries(GWmodelDesktop 
    PRIVATE 
    Qt5::Xml 
    Qt5::PrintSupport 
    Qt5::Network
    Qt5::Gui 
    Qt5::Svg 
    Qt5::Core 
    ${ARMADILLO_LIBRARIES}
    ${BLAS_LIBRARIES}
    ${GSL_LIBRARIES}
    ${QSCINTILLA_LIBRARY}
    ${QGIS_GUI_LIBRARY}
    ${QGIS_CORE_LIBRARY}
    ${QWT_LIBRARY}
    ${GDAL_LIBRARY}
    ${GEOS_LIBRARY}
    ${SQLITE3_LIBRARY}
    ${OpenMP_C_LIBRARIES}
    ${OpenMP_CXX_LIBRARIES}
)

if(ENABLE_CUDA)
target_link_libraries(GWmodelDesktop 
    PRIVATE 
    ${GWMODELCUDA_LIBRARIES}
)
endif(ENABLE_CUDA)
