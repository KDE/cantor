set( ScilabBackend_SRCS
  scilabbackend.cpp
  scilabsession.cpp
  scilabexpression.cpp
  scilabextensions.cpp
  scilabkeywords.cpp
  scilabhighlighter.cpp
  scilabcompletionobject.cpp
)

kconfig_add_kcfg_files(ScilabBackend_SRCS settings.kcfgc)
install(FILES scilabbackend.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

ki18n_wrap_ui(ScilabBackend_SRCS settings.ui)

add_backend(scilabbackend ${ScilabBackend_SRCS})

target_link_libraries(cantor_scilabbackend
  KF5::KIOCore
  KF5::SyntaxHighlighting
)

install(FILES cantor_scilab.knsrc  DESTINATION  ${KDE_INSTALL_CONFDIR})
