find_package(KDE4 REQUIRED KIO KDIRWATCH)

set( ScilabBackend_SRCS
     scilabbackend.cpp
     scilabsession.cpp
     scilabexpression.cpp
     scilabextensions.cpp
     scilabkeywords.cpp
     scilabhighlighter.cpp
     scilabcompletionobject.cpp
)

kde4_add_kcfg_files(ScilabBackend_SRCS settings.kcfgc)
install(FILES scilabbackend.kcfg DESTINATION ${KCFG_INSTALL_DIR})

kde4_add_ui_files(ScilabBackend_SRCS settings.ui)

kde4_add_plugin( cantor_scilabbackend ${ScilabBackend_SRCS} )
target_link_libraries( cantor_scilabbackend ${KDE4_KDEUI_LIBS} cantorlibs ${KDE4_KIO_LIBS})

install(FILES scilabbackend.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install(FILES keywords.xml DESTINATION ${DATA_INSTALL_DIR}/cantor/scilabbackend)
install(TARGETS cantor_scilabbackend DESTINATION ${PLUGIN_INSTALL_DIR})
