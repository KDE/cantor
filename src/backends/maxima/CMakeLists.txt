set( MaximaBackend_SRCS
  maximabackend.cpp
  maximasession.cpp
  maximaexpression.cpp
  maximaextensions.cpp
  maximahighlighter.cpp
  maximakeywords.cpp
  maximacompletionobject.cpp
  maximasyntaxhelpobject.cpp
  maximavariablemodel.cpp
)

kconfig_add_kcfg_files(MaximaBackend_SRCS settings.kcfgc)
install(FILES maximabackend.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

ki18n_wrap_ui(MaximaBackend_SRCS settings.ui)

add_backend(maximabackend ${MaximaBackend_SRCS})

if(NOT WIN32)
  target_link_libraries(cantor_maximabackend KF5::Pty KF5::SyntaxHighlighting)
endif(NOT WIN32)

if(BUILD_TESTING)
  add_executable( testmaxima testmaxima.cpp)
  add_test(NAME testmaxima COMMAND testmaxima)
  target_link_libraries( testmaxima
    Qt5::Test
    cantorlibs
    cantortest
  )
endif()

install( FILES cantor_maxima.knsrc  DESTINATION  ${KDE_INSTALL_CONFDIR} )
install( FILES cantor-initmaxima.lisp DESTINATION ${KDE_INSTALL_DATADIR}/cantor/maximabackend)
