find_package(KDE4 REQUIRED KIO KDIRWATCH)

set( SageBackend_SRCS 
 sagebackend.cpp  
 sagesession.cpp
 sageexpression.cpp
 sageextensions.cpp
 sagehighlighter.cpp
 sagetabcompletionobject.cpp
)

kde4_add_kcfg_files(SageBackend_SRCS settings.kcfgc)
install(FILES sagebackend.kcfg DESTINATION ${KCFG_INSTALL_DIR})

kde4_add_ui_files(SageBackend_SRCS settings.ui)

kde4_add_plugin( mathematik_sagebackend ${SageBackend_SRCS} )
target_link_libraries( mathematik_sagebackend ${KDE4_KDEUI_LIBS} mathematiklibs kpty ${KDE4_KIO_LIBS})

install( FILES sagebackend.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/mathematik)
install(TARGETS mathematik_sagebackend DESTINATION ${PLUGIN_INSTALL_DIR})
