find_package(KDE4 REQUIRED KIO KDIRWATCH)

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

kde4_add_kcfg_files(MaximaBackend_SRCS settings.kcfgc)
install(FILES maximabackend.kcfg DESTINATION ${KCFG_INSTALL_DIR})

kde4_add_ui_files(MaximaBackend_SRCS settings.ui)

kde4_add_plugin( cantor_maximabackend ${MaximaBackend_SRCS} )
target_link_libraries( cantor_maximabackend ${KDE4_KDEUI_LIBS} cantorlibs ${KDE4_KIO_LIBS})
if(NOT WIN32)
   target_link_libraries(cantor_maximabackend ${KDE4_KPTY_LIBS})
endif(NOT WIN32)

kde4_add_unit_test( testmaxima testmaxima.cpp)
target_link_libraries( testmaxima
  cantorlibs
  cantortest
  ${QT_QTCORE_LIBRARY}
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBS}
)

install( FILES maximabackend.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install(TARGETS cantor_maximabackend DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES keywords.xml cantor-initmaxima.lisp DESTINATION ${DATA_INSTALL_DIR}/cantor/maximabackend)
