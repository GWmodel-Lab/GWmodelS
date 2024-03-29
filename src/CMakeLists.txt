cmake_minimum_required(VERSION 3.12.0 FATAL_ERROR)

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

find_package(HDF5)
if(HDF5_FOUND)
    include_directories(${HDF5_INCLUDE_DIRS})
    set(ADDITIONAL_HDF5_LIBRARIES
        ${HDF5_LIBRARIES}
        ${HDF5_HL_LIBRARIES}
    )
else(HDF5_FOUND)
    set(ADDITIONAL_HDF5_LIBRARIES "")
endif(HDF5_FOUND)

find_package(Qwt REQUIRED)
if(NOT QWT_FOUND)
    message(FATAL_ERROR "Qwt Not Found!")
endif(NOT QWT_FOUND)
include_directories(${QWT_INCLUDE_DIR})

option(ENABLE_OpenMP on)
if(ENABLE_OpenMP)
    add_definitions(-D ENABLE_OpenMP)
    find_package(OpenMP REQUIRED)
    if(NOT OpenMP_FOUND)
        message(FATAL_ERROR "OpenMP Not Found!")
    endif(NOT OpenMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS"${CMAKE_EXE_LINKER_FLAGS}${OpenMP_EXE_LINKER_FLAGS}")
    include_directories(${OpenMP_C_INCLUDE_DIRS} ${OpenMP_CXX_INCLUDE_DIRS})
endif(ENABLE_OpenMP)

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

find_package(GWmodel REQUIRED)
include_directories(${GWmodel_INCLUDE_DIRS})

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
    gwmapp.cpp
    gwmattributetabledialog.cpp
    gwmcoordtranssettingdialog.cpp
    gwmcsvtodatdialog.cpp
    gwmdelimitedtextfile.cpp
    gwmfeaturepanel.cpp
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
    gwmrobustgwroptionsdialog.cpp
    gwmsaveascsvdialog.cpp
    gwmscalablegwroptionsdialog.cpp
    gwmtoolbar.cpp
    main.cpp
    aboutdevelopteam.cpp
    aboutdevelopers.cpp
    gwmprojcrssettingdialog.cpp
    gwmgwaverageoptionsdialog.cpp
    gwmcorrelationdialog.cpp
)

set(HEADERS
    gwmapp.h
    gwmattributetabledialog.h
    gwmcoordtranssettingdialog.h
    gwmcsvtodatdialog.h
    gwmdelimitedtextfile.h
    gwmenumvaluenamemapper.h
    gwmfeaturepanel.h
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
    gwmrobustgwroptionsdialog.h
    gwmsaveascsvdialog.h
    gwmscalablegwroptionsdialog.h
    gwmtoolbar.h
    prefix.h
    aboutdevelopteam.h
    aboutdevelopers.h
    gwmprojcrssettingdialog.h
    gwmgwaverageoptionsdialog.h
    gwmcorrelationdialog.h
)

set(FORMS
    gwmapp.ui
    gwmcoordtranssettingdialog.ui
    gwmcsvtodatdialog.ui
    gwmfeaturepanel.ui
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
    aboutdevelopteam.ui
    aboutdevelopers.ui
    gwmprojcrssettingdialog.ui
    gwmgwaverageoptionsdialog.ui
    gwmcorrelationdialog.ui
)

add_subdirectory("./AttributeTable")
add_subdirectory("./DelimitedText")
add_subdirectory("./GWmodel")
add_subdirectory("./Layout")
add_subdirectory("./Model")
add_subdirectory("./PropertyPanelTabs")
add_subdirectory("./SpatialWeight")
add_subdirectory("./SymbolWindow")
add_subdirectory("./TaskThread")
add_subdirectory("./Validity")

qt5_wrap_ui(FORMS_H ${FORMS})

if(WIN32)
    add_executable(GWmodelS WIN32 ${SOURCES} ${HEADERS} ${FORMS_H} ${app_WINRC} ${GWMODEL_ICONS})
endif(WIN32)

if(UNIX AND NOT CYGWIN)
    if(APPLE)
        add_executable(GWmodelS MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${FORMS_H} ${app_MACICON} ${GWMODEL_ICONS})
        set_target_properties(GWmodelS PROPERTIES FRAMEWORK TRUE MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/macOS/Info.plist.in)
    else(APPLE)
        set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -no-pie")
        add_executable(GWmodelS ${SOURCES} ${HEADERS} ${FORMS_H} ${app_WINRC} ${GWMODEL_ICONS})
        set_target_properties(GWmodelS PROPERTIES POSITION_INDEPENDENT_CODE FALSE)
    endif(APPLE)    
endif(UNIX AND NOT CYGWIN)

if(CYGWIN)
    message(FATAL_ERROR "Not support Cygwin currently!")
endif(CYGWIN)

target_link_libraries(GWmodelS 
    PRIVATE 
    Qt5::Xml 
    Qt5::PrintSupport 
    Qt5::Network
    Qt5::Gui 
    Qt5::Svg 
    Qt5::Core 
    ${BLAS_LIBRARIES}
    ${LAPACK_LIBRARIES}
    ${ARMADILLO_LIBRARIES}
    ${GDAL_LIBRARY}
    ${GEOS_LIBRARY}
    ${SQLITE3_LIBRARY}
    ${OpenMP_C_LIBRARIES}
    ${OpenMP_CXX_LIBRARIES}
    ${GSL_LIBRARIES}
    ${QSCINTILLA_LIBRARY}
    ${QWT_LIBRARY}
    ${QGIS_GUI_LIBRARY}
    ${QGIS_CORE_LIBRARY}
    ${ADDITIONAL_LIBRARIES}
    ${GWmodel_LIBRARIES}
)

if(ENABLE_CUDA)
target_link_libraries(GWmodelS 
    PRIVATE 
    ${GWMODELCUDA_LIBRARIES}
)
endif(ENABLE_CUDA)