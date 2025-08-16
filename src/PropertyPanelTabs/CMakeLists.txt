get_filename_component(SRC_SUBFOLDER_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set(SOURCES_SUBFOLDER
    gwmpropertycollinearitygwrtab.cpp
    gwmpropertyggwrtab.cpp
    gwmpropertygtwrtab.cpp
    gwmpropertygwpcatab.cpp
    gwmpropertygwrtab.cpp
    gwmpropertygtdrtab.cpp
    gwmpropertymultiscalegwrtab.cpp
    gwmpropertyscalablegwrtab.cpp
    gwmpropertydefaulttab.cpp
    gwmpropertystatisticstab.cpp
    gwmpropertygwaveragetab.cpp
    gwmpropertygwcorrelationstab.cpp
)

set(HEADERS_SUBFOLDER
    gwmpropertycollinearitygwrtab.h
    gwmpropertyggwrtab.h
    gwmpropertygtwrtab.h
    gwmpropertygwpcatab.h
    gwmpropertygwrtab.h
    gwmpropertygtdrtab.h
    gwmpropertymultiscalegwrtab.h
    gwmpropertyscalablegwrtab.h
    gwmpropertydefaulttab.h
    gwmpropertystatisticstab.h
    gwmpropertygwaveragetab.h
    gwmpropertygwcorrelationstab.h
    utils.h
)

set(FORMS_SUBFOLDER
    gwmpropertycollinearitygwrtab.ui
    gwmpropertyggwrtab.ui
    gwmpropertygtwrtab.ui
    gwmpropertygwpcatab.ui
    gwmpropertygwrtab.ui
    gwmpropertygtdrtab.ui
    gwmpropertymultiscalegwrtab.ui
    gwmpropertyscalablegwrtab.ui
    gwmpropertydefaulttab.ui
    gwmpropertystatisticstab.ui
    gwmpropertygwaveragetab.ui
    gwmpropertygwcorrelationstab.ui
)

list(TRANSFORM SOURCES_SUBFOLDER PREPEND "${SRC_SUBFOLDER_NAME}/")
list(TRANSFORM HEADERS_SUBFOLDER PREPEND "${SRC_SUBFOLDER_NAME}/")
list(TRANSFORM FORMS_SUBFOLDER PREPEND "${SRC_SUBFOLDER_NAME}/")

set(SOURCES ${SOURCES_SUBFOLDER} ${SOURCES} PARENT_SCOPE)
set(HEADERS ${HEADERS_SUBFOLDER} ${HEADERS} PARENT_SCOPE)
set(FORMS ${FORMS_SUBFOLDER} ${FORMS} PARENT_SCOPE)
