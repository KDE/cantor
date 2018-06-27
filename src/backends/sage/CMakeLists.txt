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

add_backend(sagebackend ${SageBackend_SRCS})

target_link_libraries( cantor_sagebackend KF5::Pty)

if(BUILD_TESTING)
  add_executable( testsage testsage.cpp)
  target_link_libraries( testsage
    ${QT_QTTEST_LIBRARY}
    cantorlibs
    cantortest
  )

  add_test(NAME testsage COMMAND testsage)
endif()

install(FILES cantor_sage.knsrc  DESTINATION  ${KDE_INSTALL_CONFDIR})

install(
  FILES cantor-execsage
  PERMISSIONS OWNER_READ, OWNER_WRITE, OWNER_EXECUTE, GROUP_READ, GROUP_WRITE, GROUP_EXECUTE, WORLD_READ, WORLD_EXECUTE
  DESTINATION ${KDE_INSTALL_DATADIR}/cantor/sagebackend
)
