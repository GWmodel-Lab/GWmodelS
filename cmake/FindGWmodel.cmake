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
# ``GWMODEL_FOUND`` True if GWmodel found on the local system
#
# ``GWMODEL_INCLUDE_DIR`` Location of GWmodel header files
#
# ``GWMODEL_LIBRARY_DIR`` Location of GWmodel libraries
#
# ``GWMODEL_LIBRARIES`` List of the GWmodel libraries found
#

find_file(GWMODEL_INCLUDE_FILE
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

find_library(GWMODEL_LIBRARIES
    NAMES gwmodel.lib libgwmodel.a
    PATHS
    /usr/lib
    /usr/local/lib
    /usr/local/gwmodel/lib
    "$ENV{LIB_DIR}/lib"
    "$ENV{LIB}/lib"
    "$ENV{USR_LOCAL}/lib"
)

if(GWMODEL_INCLUDE_FILE)
    get_filename_component(GWMODEL_INCLUDE_DIR ${GWMODEL_INCLUDE_FILE} DIRECTORY CACHE)
    set(GWMODEL_INCLUDE_FOUDN TRUE)
endif(GWMODEL_INCLUDE_FILE)


if(GWMODEL_LIBRARIES)
    set(GWMODEL_LIBRARIES_FOUND TRUE)
endif()

if(GWMODEL_INCLUDE_FOUDN AND GWMODEL_LIBRARIES_FOUND)
    set(GWMODEL_FOUND TRUE)
endif()

if(GWMODEL_FOUND)
    message(STATUS "Found GWmodel: ${GWMODEL_LIBRARIES}")
else()
    if(GWMODEL_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find GWmodel")
    endif(GWMODEL_FIND_REQUIRED)
endif()
