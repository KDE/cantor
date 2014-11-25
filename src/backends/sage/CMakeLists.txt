set( SageBackend_SRCS
 sagebackend.cpp
 sagesession.cpp
 sageexpression.cpp
 sageextensions.cpp
 sagekeywords.cpp
 sagehighlighter.cpp
 sagecompletionobject.cpp
)

kconfig_add_kcfg_files(SageBackend_SRCS settings.kcfgc)
install(FILES sagebackend.kcfg DESTINATION ${KCFG_INSTALL_DIR})

ki18n_wrap_ui(SageBackend_SRCS settings.ui)

add_library( cantor_sagebackend MODULE ${SageBackend_SRCS} )
target_link_libraries( cantor_sagebackend cantorlibs KF5::Pty)

add_executable( testsage testsage.cpp)
target_link_libraries( testsage
  ${QT_QTTEST_LIBRARY}
  cantorlibs
  cantortest
)

install( FILES cantor_sage.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )
install( FILES sagebackend.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install(TARGETS cantor_sagebackend DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES keywords.xml DESTINATION ${DATA_INSTALL_DIR}/cantor/sagebackend)
