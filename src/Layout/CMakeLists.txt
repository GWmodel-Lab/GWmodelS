get_filename_component(SRC_SUBFOLDER_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set(SOURCES_SUBFOLDER
    gwmlayoutbatchconfigurationdelegate.cpp
    gwmlayoutbatchconfigurationmodel.cpp
    gwmlayoutbatchdialog.cpp
    gwmlayoutbatchfieldlistmodel.cpp
    gwmlayoutbatchlayerlistmodel.cpp
    gwmlayoutbatchunifylayerpopupdialog.cpp
    gwmlayoutdesigner.cpp
    qgslayoutmanagerdialog.cpp
)

set(HEADERS_SUBFOLDER
    gwmlayoutbatchconfigurationdelegate.h
    gwmlayoutbatchconfigurationmodel.h
    gwmlayoutbatchdialog.h
    gwmlayoutbatchfieldlistmodel.h
    gwmlayoutbatchlayerlistmodel.h
    gwmlayoutbatchunifylayerpopupdialog.h
    gwmlayoutdesigner.h
    qgslayoutmanagerdialog.h
)

set(FORMS_SUBFOLDER
    gwmlayoutbatchdialog.ui
    gwmlayoutbatchunifylayerpopupdialog.ui
    gwmlayoutdesigner.ui
    qgslayoutatlaswidgetbase.ui
    qgslayoutmanagerbase.ui
    qgssvgexportoptions.ui
)

list(TRANSFORM SOURCES_SUBFOLDER PREPEND "${SRC_SUBFOLDER_NAME}/")
list(TRANSFORM HEADERS_SUBFOLDER PREPEND "${SRC_SUBFOLDER_NAME}/")
list(TRANSFORM FORMS_SUBFOLDER PREPEND "${SRC_SUBFOLDER_NAME}/")

set(SOURCES ${SOURCES_SUBFOLDER} ${SOURCES} PARENT_SCOPE)
set(HEADERS ${HEADERS_SUBFOLDER} ${HEADERS} PARENT_SCOPE)
set(FORMS ${FORMS_SUBFOLDER} ${FORMS} PARENT_SCOPE)
