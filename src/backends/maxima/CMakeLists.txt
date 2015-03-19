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

add_library( cantor_maximabackend MODULE ${MaximaBackend_SRCS} )
target_link_libraries(cantor_maximabackend cantorlibs)

if(NOT WIN32)
  target_link_libraries(cantor_maximabackend KF5::Pty)
endif(NOT WIN32)

add_executable( testmaxima testmaxima.cpp)
target_link_libraries( testmaxima
  cantorlibs
  cantortest
  ${QTTEST_LIBRARY}
)

install( FILES cantor_maxima.knsrc  DESTINATION  ${KDE_INSTALL_CONFDIR} )
install( FILES maximabackend.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/cantor)
install(TARGETS cantor_maximabackend DESTINATION ${KDE_INSTALL_PLUGINDIR})
install( FILES keywords.xml cantor-initmaxima.lisp DESTINATION ${KDE_INSTALL_DATADIR}/cantor/maximabackend)
