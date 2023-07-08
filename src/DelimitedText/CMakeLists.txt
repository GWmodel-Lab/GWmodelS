get_filename_component(SRC_SUBFOLDER_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set(SOURCES_SUBFOLDER
    qgsdelimitedtextfeatureiterator.cpp
    qgsdelimitedtextfile.cpp
    qgsdelimitedtextprovider.cpp
)

set(HEADERS_SUBFOLDER
    qgsdelimitedtextfeatureiterator.h
    qgsdelimitedtextfile.h
    qgsdelimitedtextprovider.h
)

list(TRANSFORM SOURCES_SUBFOLDER PREPEND "${SRC_SUBFOLDER_NAME}/")
list(TRANSFORM HEADERS_SUBFOLDER PREPEND "${SRC_SUBFOLDER_NAME}/")

set(SOURCES ${SOURCES_SUBFOLDER} ${SOURCES} PARENT_SCOPE)
set(HEADERS ${HEADERS_SUBFOLDER} ${HEADERS} PARENT_SCOPE)
