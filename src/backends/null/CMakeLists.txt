set( NullBackend_SRCS
 nullbackend.cpp
 nullsession.cpp
 nullexpression.cpp
 nullcompletionobject.cpp
)

add_library(cantor_nullbackend MODULE ${NullBackend_SRCS} )
target_link_libraries(cantor_nullbackend cantorlibs)

install( FILES nullbackend.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install(TARGETS cantor_nullbackend DESTINATION ${PLUGIN_INSTALL_DIR})
