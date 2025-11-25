get_filename_component(SRC_SUBFOLDER_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set(SOURCES_SUBFOLDER
    gwmbandwidthsizeselector.cpp
    gwmbasicgwralgorithm.cpp
    gwmcsvtodatthread.cpp
    gwmgeneralizedgwralgorithm.cpp
    gwmgeographicalweightedregressionalgorithm.cpp
    gwmgtwralgorithm.cpp
    gwmgwpcataskthread.cpp
    gwmggwrbandwidthsizeselector.cpp
    gwmgtdrtaskthread.cpp
    gwmindependentvariableselector.cpp
    gwmlocalcollinearitygwralgorithm.cpp
    gwmmultiscalegwralgorithm.cpp
    gwmrobustgwralgorithm.cpp
    gwmsavelayerthread.cpp
    gwmscalablegwralgorithm.cpp
    gwmspatialalgorithm.cpp
    gwmspatialmonoscalealgorithm.cpp
    gwmspatialmultiscalealgorithm.cpp
    gwmspatialtemporalmonoscale.cpp
    gwmcoordtransthread.cpp
    gwmtaskthread.cpp
    gwmgwaveragetaskthread.cpp
    gwmgwcorrelationtaskthread.cpp
)

set(HEADERS_SUBFOLDER
    gwmbandwidthsizeselector.h
    gwmbasicgwralgorithm.h
    gwmgeneralizedgwralgorithm.h
    gwmgeographicalweightedregressionalgorithm.h
    gwmgtwralgorithm.h
    gwmgwpcataskthread.h
    gwmggwrbandwidthsizeselector.h
    gwmgtdrtaskthread.h
    gwmindependentvariableselector.h
    gwmlocalcollinearitygwralgorithm.h
    gwmmultiscalegwralgorithm.h
    gwmrobustgwralgorithm.h
    gwmrobustgwralgorithm.h
    gwmsavelayerthread.h
    gwmscalablegwralgorithm.h
    gwmspatialalgorithm.h
    gwmspatialmonoscalealgorithm.h
    gwmspatialmultiscalealgorithm.h
    gwmspatialtemporalmonoscale.h
    imonovariableanalysis.h
    imultivariableanalysis.h
    iparallelable.h
    iregressionanalysis.h
    gwmcoordtransthread.h
    gwmtaskthread.h
    gwmgwaveragetaskthread.h
    gwmgwcorrelationtaskthread.h
)

list(TRANSFORM SOURCES_SUBFOLDER PREPEND "${SRC_SUBFOLDER_NAME}/")
list(TRANSFORM HEADERS_SUBFOLDER PREPEND "${SRC_SUBFOLDER_NAME}/")

set(SOURCES ${SOURCES_SUBFOLDER} ${SOURCES} PARENT_SCOPE)
set(HEADERS ${HEADERS_SUBFOLDER} ${HEADERS} PARENT_SCOPE)
