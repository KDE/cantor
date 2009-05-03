set( NullBackend_SRCS 
 nullbackend.cpp  
 nullsession.cpp
 nullexpression.cpp
)

kde4_add_plugin( mathematik_nullbackend ${NullBackend_SRCS} )
target_link_libraries( mathematik_nullbackend ${KDE4_KDEUI_LIBS} mathematiklibs)

install( FILES nullbackend.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/mathematik)
install(TARGETS mathematik_nullbackend DESTINATION ${PLUGIN_INSTALL_DIR})
