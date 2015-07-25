find_package(KDE4 REQUIRED KIO KDIRWATCH)

set( SageBackend_SRCS
 sagebackend.cpp
 sagesession.cpp
 sageexpression.cpp
 sageextensions.cpp
 sagekeywords.cpp
 sagehighlighter.cpp
 sagecompletionobject.cpp
)

kde4_add_kcfg_files(SageBackend_SRCS settings.kcfgc)
install(FILES sagebackend.kcfg DESTINATION ${KCFG_INSTALL_DIR})

kde4_add_ui_files(SageBackend_SRCS settings.ui)

kde4_add_plugin( cantor_sagebackend ${SageBackend_SRCS} )
target_link_libraries( cantor_sagebackend ${KDE4_KDEUI_LIBS} cantorlibs ${KDE4_KPTY_LIBS} ${KDE4_KIO_LIBS})

kde4_add_unit_test( testsage testsage.cpp)
target_link_libraries( testsage
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
  cantorlibs
  cantortest
)

install( FILES sagebackend.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install(TARGETS cantor_sagebackend DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES keywords.xml DESTINATION ${DATA_INSTALL_DIR}/cantor/sagebackend)
