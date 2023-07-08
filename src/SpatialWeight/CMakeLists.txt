get_filename_component(SRC_SUBFOLDER_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set(SOURCES_SUBFOLDER
    gwmbandwidthweight.cpp
    gwmcrsdistance.cpp
    gwmdistance.cpp
    gwmdmatdistance.cpp
    gwmminkwoskidistance.cpp
    gwmspatialtemporalweight.cpp
    gwmspatialweight.cpp
    gwmweight.cpp
)

set(HEADERS_SUBFOLDER
    gwmbandwidthweight.h
    gwmcrsdistance.h
    gwmdistance.h
    gwmdmatdistance.h
    gwmminkwoskidistance.h
    gwmspatialtemporalweight.h
    gwmspatialweight.h
    gwmweight.h
)

list(TRANSFORM SOURCES_SUBFOLDER PREPEND "${SRC_SUBFOLDER_NAME}/")
list(TRANSFORM HEADERS_SUBFOLDER PREPEND "${SRC_SUBFOLDER_NAME}/")

set(SOURCES ${SOURCES_SUBFOLDER} ${SOURCES} PARENT_SCOPE)
set(HEADERS ${HEADERS_SUBFOLDER} ${HEADERS} PARENT_SCOPE)
