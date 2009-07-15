find_package(R)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")

include_directories(${R_INCLUDEDIR} rinside rcpp)
LINK_DIRECTORIES(${R_SHAREDLIBDIR})

set( RBackend_SRCS 
 rbackend.cpp  
 rsession.cpp
 rexpression.cpp
 rthread.cpp
 rcallbacks.cpp
)

kde4_add_plugin( mathematik_rbackend ${RBackend_SRCS} )
target_link_libraries( mathematik_rbackend ${KDE4_KDEUI_LIBS} mathematiklibs rinside rcpp ${R_USED_LIBS})

install( FILES rbackend.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/mathematik)
install(TARGETS mathematik_rbackend DESTINATION ${PLUGIN_INSTALL_DIR})
