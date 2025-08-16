get_filename_component(SRC_SUBFOLDER_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set(SOURCES_SUBFOLDER
    gwmlayerattributeitem.cpp
    gwmlayerattributeitemmodel.cpp
    gwmlayerbasicgwritem.cpp
    gwmlayercollinearitygwritem.cpp
    gwmlayerggwritem.cpp
    gwmlayergroupitem.cpp
    gwmlayergtwritem.cpp
    gwmlayergwpcaitem.cpp
    gwmlayergwssitem.cpp
    gwmlayergwaverageitem.cpp
    gwmlayergtdritem.cpp
    gwmlayeritem.cpp
    gwmlayeritemmodel.cpp
    gwmlayermultiscalegwritem.cpp
    gwmlayeroriginitem.cpp
    gwmlayerscalablegwritem.cpp
    gwmlayersymbolitem.cpp
    gwmlayervectoritem.cpp
    gwmvariableitemmodel.cpp
    gwmalgorithmmetagtdr.cpp
    gwmalgorithmmetavariable.cpp
    gwmparameterspecifiedoptionsmodel.cpp
    gwmpropertymultiscaleparameterspecifieditemmodel.cpp
    gwmpropertygwcorrelationsparameterspecifieditemmodel.cpp
)

set(HEADERS_SUBFOLDER
    gwmlayerattributeitem.h
    gwmlayerattributeitemmodel.h
    gwmlayerbasicgwritem.h
    gwmlayercollinearitygwritem.h
    gwmlayerggwritem.h
    gwmlayergroupitem.h
    gwmlayergtwritem.h
    gwmlayergwpcaitem.h
    gwmlayergwssitem.h
    gwmlayergwaverageitem.h
    gwmlayergtdritem.h
    gwmlayeritem.h
    gwmlayeritemmodel.h
    gwmlayermultiscalegwritem.h
    gwmlayeroriginitem.h
    gwmlayerscalablegwritem.h
    gwmlayersymbolitem.h
    gwmlayervectoritem.h
    gwmvariableitemmodel.h
    gwmalgorithmmetagtdr.h
    gwmalgorithmmetavariable.h
    gwmparameterspecifiedoptionsmodel.h
    gwmpropertymultiscaleparameterspecifieditemmodel.h
    gwmpropertygwcorrelationsparameterspecifieditemmodel.h
)

list(TRANSFORM SOURCES_SUBFOLDER PREPEND "${SRC_SUBFOLDER_NAME}/")
list(TRANSFORM HEADERS_SUBFOLDER PREPEND "${SRC_SUBFOLDER_NAME}/")

set(SOURCES ${SOURCES_SUBFOLDER} ${SOURCES} PARENT_SCOPE)
set(HEADERS ${HEADERS_SUBFOLDER} ${HEADERS} PARENT_SCOPE)
