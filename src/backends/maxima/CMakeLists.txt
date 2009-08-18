find_package(KDE4 REQUIRED KIO KDIRWATCH)

set( MaximaBackend_SRCS 
 maximabackend.cpp  
 maximasession.cpp
 maximaexpression.cpp
 maximaextensions.cpp
 maximahighlighter.cpp
)

kde4_add_kcfg_files(MaximaBackend_SRCS settings.kcfgc)
install(FILES maximabackend.kcfg DESTINATION ${KCFG_INSTALL_DIR})

kde4_add_ui_files(MaximaBackend_SRCS settings.ui)

kde4_add_plugin( mathematik_maximabackend ${MaximaBackend_SRCS} )
target_link_libraries( mathematik_maximabackend ${KDE4_KDEUI_LIBS} mathematiklibs kpty ${KDE4_KIO_LIBS})

install( FILES maximabackend.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/mathematik)
install(TARGETS mathematik_maximabackend DESTINATION ${PLUGIN_INSTALL_DIR})
