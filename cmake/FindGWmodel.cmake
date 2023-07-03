# FindGWmodel
# --------
#
# Find the GWmodel libraries
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# The following variables will be defined:
#
# ``GWmodel_FOUND`` True if GWmodel found on the local system
#
# ``GWmodel_INCLUDE_DIR`` Location of GWmodel header files
#
# ``GWmodel_LIBRARY_DIR`` Location of GWmodel libraries
#
# ``GWmodel_LIBRARIES`` List of the GWmodel libraries found
#

find_path(GWmodel_INCLUDE_FILE
    NAMES gwmodel.h
    PATHS 
    /usr/include
    /usr/include/gwmodel
    /usr/local/include
    /usr/local/include/gwmodel
    /usr/local/gwmodel/include
    "$ENV{LIB_DIR}/include"
    "$ENV{INCLUDE}"
)

find_library(GWmodel_LIBRARIES
    NAMES gwmodel.lib libgwmodel.a
    PATHS
    /usr/lib
    /usr/local/lib
    /usr/local/gwmodel/lib
    "$ENV{LIB_DIR}/lib"
    "$ENV{LIB}/lib"
    "$ENV{USR_LOCAL}/lib"
)

if(GWmodel_INCLUDE_FILE)
    get_filename_component(GWmodel_INCLUDE_DIR ${GWmodel_INCLUDE_FILE} DIRECTORY)
    set(GWmodel_INCLUDE_FOUDN TRUE)
endif(GWmodel_INCLUDE_FILE)


if(GWmodel_LIBRARIES)
    set(GWmodel_LIBRARIES_FOUND TRUE)
endif()

if(GWmodel_INCLUDE_FOUDN AND GWmodel_LIBRARIES_FOUND)
    set(GWmodel_FOUND TRUE)
endif()

if(GWmodel_FOUND)
    message(STATUS "Found GWmodel: ${GWmodel_LIBRARIES}")
else()
    if(GWmodel_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find GWmodel")
    endif(GWmodel_FIND_REQUIRED)
endif()
