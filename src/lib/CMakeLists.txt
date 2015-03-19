set(GENERIC_LIB_VERSION "0.6.0")
set(GENERIC_LIB_SOVERSION "6")

set( cantor_LIB_SRCS
  session.cpp
  expression.cpp
  backend.cpp
  result.cpp
  textresult.cpp
  imageresult.cpp
  epsresult.cpp
  latexresult.cpp
  latexrenderer.cpp
  helpresult.cpp
  animationresult.cpp
  extension.cpp
  assistant.cpp
  completionobject.cpp
  syntaxhelpobject.cpp
  defaulthighlighter.cpp
  defaultvariablemodel.cpp
  panelplugin.cpp
  panelpluginhandler.cpp
  directives/plotdirectives.cpp
)

Set( cantor_LIB_HDRS
  cantor_macros.h
  cantor_export.h
  #base classes
  backend.h
  session.h
  expression.h
  extension.h
  syntaxhelpobject.h
  completionobject.h
  #results
  animationresult.h
  epsresult.h
  helpresult.h
  imageresult.h
  latexresult.h
  result.h
  textresult.h
  #helper classes
  defaulthighlighter.h
  defaultvariablemodel.h

)

ki18n_wrap_ui(cantor_LIB_SRCS directives/axisrange.ui directives/plottitle.ui)

kconfig_add_kcfg_files(cantor_LIB_SRCS settings.kcfgc)
install(FILES cantor_libs.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

configure_file (config-cantorlib.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-cantorlib.h )

add_library( cantorlibs  SHARED ${cantor_LIB_SRCS} )

target_link_libraries( cantorlibs
  KF5::KDELibs4Support
  KF5::KIOCore
  KF5::Archive
  KF5::ConfigCore
  KF5::ConfigGui
  ${QT5_LIBRARIES}
  Qt5::Xml
)

set_target_properties( cantorlibs
  PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION}
)


install( TARGETS cantorlibs  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

install( FILES ${cantor_LIB_HDRS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR}/cantor
  COMPONENT Devel
)

install( FILES cantor_backend.desktop  DESTINATION ${KDE_INSTALL_KSERVICETYPES5DIR})
install( FILES cantor_assistant.desktop  DESTINATION ${KDE_INSTALL_KSERVICETYPES5DIR})
install( FILES cantor_panelplugin.desktop  DESTINATION ${KDE_INSTALL_KSERVICETYPES5DIR})

add_subdirectory(test)
