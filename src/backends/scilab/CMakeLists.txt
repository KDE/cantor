set( ScilabBackend_SRCS
  scilabbackend.cpp
  scilabsession.cpp
  scilabexpression.cpp
  scilabextensions.cpp
  scilabkeywords.cpp
  scilabhighlighter.cpp
  scilabcompletionobject.cpp
  scilabsettingswidget.cpp
  ../backendsettingswidget.cpp
)

kconfig_add_kcfg_files(ScilabBackend_SRCS settings.kcfgc)
install(FILES scilabbackend.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

ki18n_wrap_ui(ScilabBackend_SRCS settings.ui)

add_backend(scilabbackend ${ScilabBackend_SRCS})

target_link_libraries(cantor_scilabbackend
  KF5::KIOCore
  KF5::SyntaxHighlighting
)

if (ENABLE_EMBEDDED_DOCUMENTATION)
    target_link_libraries(cantor_scilabbackend cantor_help)
endif ()

if(BUILD_TESTING)
  add_executable(testscilab testscilab.cpp)
  add_test(NAME testscilab COMMAND testscilab)
  target_link_libraries(testscilab
    Qt5::Test
    cantorlibs
    cantortest
  )
endif(BUILD_TESTING)

install(FILES cantor_scilab.knsrc  DESTINATION  ${KDE_INSTALL_KNSRCDIR})
