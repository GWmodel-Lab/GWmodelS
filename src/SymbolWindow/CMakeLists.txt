get_filename_component(SRC_SUBFOLDER_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set(SOURCES_SUBFOLDER
    gwmsymboleditordialog.cpp
    gwmsymbolwindow.cpp
)

set(HEADERS_SUBFOLDER
    gwmsymboleditordialog.h
    gwmsymbolwindow.h
)

set(FORMS_SUBFOLDER
    qgsgraduatedsymbolrendererwidget.ui
    qgscategorizedsymbolrendererwidget.ui
    qgsheatmaprendererwidgetbase.ui
    qgshistogramwidgetbase.ui
    qgsinvertedpolygonrendererwidgetbase.ui
    qgsmapunitscalewidgetbase.ui
    qgspointclusterrendererwidgetbase.ui
    qgspointdisplacementrendererwidgetbase.ui
    qgsrendererrulepropsdialogbase.ui
    qgsrulebasedrendererwidget.ui
    qgsstyleitemslistwidgetbase.ui
    qgssymbolselectordialogbase.ui
    qgsunitselectionwidget.ui
    widget_set_dd_value.ui
)

list(TRANSFORM SOURCES_SUBFOLDER PREPEND "${SRC_SUBFOLDER_NAME}/")
list(TRANSFORM HEADERS_SUBFOLDER PREPEND "${SRC_SUBFOLDER_NAME}/")
list(TRANSFORM FORMS_SUBFOLDER PREPEND "${SRC_SUBFOLDER_NAME}/")

set(SOURCES ${SOURCES_SUBFOLDER} ${SOURCES} PARENT_SCOPE)
set(HEADERS ${HEADERS_SUBFOLDER} ${HEADERS} PARENT_SCOPE)
set(FORMS ${FORMS_SUBFOLDER} ${FORMS} PARENT_SCOPE)
