## Once run this will define:
##
## GWMODELCUDA_FOUND                = system has GWmodelCUDA lib
##
## GWMODELCUDA_INCLUDE_DIR          = full path to the GWmodelCUDA include directories, containing `GWmodelCUDA` prefix
## GWMODELCUDA_LIBRARIES            = full path to the GWmodelCUDA and CUDAInspector library
##
## The following defines may not used directly in projects
##
## GWMODELCUDA_INCLUDE_FILE_DIR     = full path to the GWmodelCUDA include directories, ending with `GWmodelCUDA`
## GWMODELCUDA_ALGORITHM_LIBRARY    = full path to the GWmodelCUDA library
## GWMODELCUDA_INSPECTOR_LIBRARY    = full path to the CUDAInspector library
##

find_path(GWMODELCUDA_INCLUDE_FILE_DIR
    NAMES IGWmodelCUDA.h ICUDAInspector.h 
    PATHS "$ENV{LIB_DIR}/include"
    "$ENV{LIB_DIR}/include/GWmodelCUDA"
    "$ENV{INCLUDE}"
    "$ENV{INCLUDE}/GWmodelCUDA"
    PATH_SUFFIXES GWmodelCUDA
)
get_filename_component(GWMODELCUDA_INCLUDE_DIR ${GWMODELCUDA_INCLUDE_FILE_DIR} DIRECTORY)

if(WIN32)
    find_library(GWMODELCUDA_ALGORITHM_LIBRARY NAMES GWmodelCUDA64
        PATHS "$ENV{LIB_DIR}/lib"
        "$ENV{LIB}/lib"
    )
else(WIN32)
    find_library(GWMODELCUDA_ALGORITHM_LIBRARY NAMES GWmodelCUDA
        PATHS "$ENV{LIB_DIR}/lib"
        "$ENV{LIB}/lib"
    )
endif(WIN32)


find_library(GWMODELCUDA_INSPECTOR_LIBRARY NAMES CUDAInspector
    PATHS "$ENV{LIB_DIR}/lib"
    "$ENV{LIB}/lib"
)

if(GWMODELCUDA_ALGORITHM_LIBRARY AND GWMODELCUDA_INSPECTOR_LIBRARY)
    set(GWMODELCUDA_LIBRARIES ${GWMODELCUDA_ALGORITHM_LIBRARY} ${GWMODELCUDA_INSPECTOR_LIBRARY})
endif(GWMODELCUDA_ALGORITHM_LIBRARY AND GWMODELCUDA_INSPECTOR_LIBRARY)

if(GWMODELCUDA_INCLUDE_DIR AND GWMODELCUDA_LIBRARIES)
    set(GWMODELCUDA_FOUND TRUE)
endif(GWMODELCUDA_INCLUDE_DIR AND GWMODELCUDA_LIBRARIES)

if(GWMODELCUDA_FOUND)
    message(STATUS "Fount GWmodelCUDA: ${GWMODELCUDA_LIBRARIES}")
else(GWMODELCUDA_FOUND)
    message(FATAL_ERROR "Could not find GWmodelCUDA")
endif(GWMODELCUDA_FOUND)

