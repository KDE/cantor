set( NullBackend_SRCS 
 nullbackend.cpp  
 nullsession.cpp
 nullexpression.cpp
 nullcompletionobject.cpp
)

kde4_add_plugin( cantor_nullbackend ${NullBackend_SRCS} )
target_link_libraries( cantor_nullbackend ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES nullbackend.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install(TARGETS cantor_nullbackend DESTINATION ${PLUGIN_INSTALL_DIR})
