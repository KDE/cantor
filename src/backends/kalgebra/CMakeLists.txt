include_directories(${analitza_SOURCE_DIR}/../)

set( KAlgebraBackend_SRCS
 kalgebrasyntaxhelpobject.cpp 
 kalgebrabackend.cpp  
 kalgebrasession.cpp
 kalgebraexpression.cpp
 kalgebratabcompletionobject.cpp
)

kde4_add_plugin( cantor_kalgebrabackend ${KAlgebraBackend_SRCS} )
target_link_libraries( cantor_kalgebrabackend ${KDE4_KDEUI_LIBS} cantorlibs analitza analitzagui)

install( FILES kalgebrabackend.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install(TARGETS cantor_kalgebrabackend DESTINATION ${PLUGIN_INSTALL_DIR})
