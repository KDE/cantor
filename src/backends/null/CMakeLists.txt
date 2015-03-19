set( NullBackend_SRCS
 nullbackend.cpp
 nullsession.cpp
 nullexpression.cpp
 nullcompletionobject.cpp
)

add_library(cantor_nullbackend MODULE ${NullBackend_SRCS} )
target_link_libraries(cantor_nullbackend cantorlibs)

install( FILES nullbackend.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/cantor)
install(TARGETS cantor_nullbackend DESTINATION ${KDE_INSTALL_PLUGINDIR})
