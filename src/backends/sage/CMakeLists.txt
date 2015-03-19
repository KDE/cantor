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
install(FILES sagebackend.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

ki18n_wrap_ui(SageBackend_SRCS settings.ui)

add_library( cantor_sagebackend MODULE ${SageBackend_SRCS} )
target_link_libraries( cantor_sagebackend cantorlibs KF5::Pty)

add_executable( testsage testsage.cpp)
target_link_libraries( testsage
  ${QT_QTTEST_LIBRARY}
  cantorlibs
  cantortest
)

install( FILES cantor_sage.knsrc  DESTINATION  ${KDE_INSTALL_CONFDIR} )
install( FILES sagebackend.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/cantor)
install(TARGETS cantor_sagebackend DESTINATION ${KDE_INSTALL_PLUGINDIR})
install( FILES keywords.xml DESTINATION ${KDE_INSTALL_DATADIR}/cantor/sagebackend)
