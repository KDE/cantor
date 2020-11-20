set( SageBackend_SRCS
  sagebackend.cpp
  sagesession.cpp
  sageexpression.cpp
  sageextensions.cpp
  sagekeywords.cpp
  sagehighlighter.cpp
  sagecompletionobject.cpp
  sagesettingswidget.cpp
  ../backendsettingswidget.cpp
)

kconfig_add_kcfg_files(SageBackend_SRCS settings.kcfgc)
install(FILES sagebackend.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

ki18n_wrap_ui(SageBackend_SRCS settings.ui)

add_backend(sagebackend ${SageBackend_SRCS})

target_link_libraries( cantor_sagebackend cantor_help
    KF5::Pty
    KF5::SyntaxHighlighting)

if(BUILD_TESTING)
  add_executable( testsage testsage.cpp)
  add_test(NAME testsage COMMAND testsage)
  target_link_libraries( testsage
    Qt5::Test
    cantorlibs
    cantortest
  )
endif()

install(FILES cantor_sage.knsrc  DESTINATION  ${KDE_INSTALL_CONFDIR})

install(
  PROGRAMS cantor-execsage
  DESTINATION ${KDE_INSTALL_DATADIR}/cantor/sagebackend
)
