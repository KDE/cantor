include_directories(${ANALITZA_INCLUDE_DIR})

set( KAlgebraBackend_SRCS
 kalgebrasyntaxhelpobject.cpp
 kalgebrabackend.cpp
 kalgebrasession.cpp
 kalgebraexpression.cpp
 kalgebracompletionobject.cpp
 kalgebraextensions.cpp
)

kde4_add_kcfg_files(KAlgebraBackend_SRCS settings.kcfgc)
install(FILES kalgebrabackend.kcfg DESTINATION ${KCFG_INSTALL_DIR})

kde4_add_ui_files(KAlgebraBackend_SRCS settings.ui)

kde4_add_plugin( cantor_kalgebrabackend ${KAlgebraBackend_SRCS} )
target_link_libraries( cantor_kalgebrabackend ${KDE4_KDEUI_LIBS} cantorlibs ${ANALITZA_LIBS})

install( FILES kalgebrabackend.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install(TARGETS cantor_kalgebrabackend DESTINATION ${PLUGIN_INSTALL_DIR})
