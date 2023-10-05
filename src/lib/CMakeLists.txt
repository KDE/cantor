if(LIBSPECTRE_FOUND)
   include_directories(${LIBSPECTRE_INCLUDE_DIR})
endif(LIBSPECTRE_FOUND)

set( cantor_LIB_SRCS
  session.cpp
  expression.cpp
  backend.cpp
  result.cpp
  textresult.cpp
  imageresult.cpp
  mimeresult.cpp
  epsresult.cpp
  latexresult.cpp
  latexrenderer.cpp
  renderer.cpp
  helpresult.cpp
  animationresult.cpp
  htmlresult.cpp
  extension.cpp
  assistant.cpp
  completionobject.cpp
  syntaxhelpobject.cpp
  defaulthighlighter.cpp
  defaultvariablemodel.cpp
  panelplugin.cpp
  panelpluginhandler.cpp
  worksheetaccess.cpp
  directives/plotdirectives.cpp
  jupyterutils.cpp
  graphicpackage.cpp
)

Set( cantor_LIB_HDRS
  cantor_macros.h
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
  renderer.h
  result.h
  textresult.h
  mimeresult.h
  htmlresult.h
  #helper classes
  defaulthighlighter.h
  defaultvariablemodel.h
  worksheetaccess.h
  jupyterutils.h
  graphicpackage.h
  # plugin classes
  panelplugin.h
  panelpluginhandler.h
)

ki18n_wrap_ui(cantor_LIB_SRCS directives/axisrange.ui directives/plottitle.ui)

kconfig_add_kcfg_files(cantor_LIB_SRCS settings.kcfgc)
install(FILES cantor_libs.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

configure_file (config-cantorlib.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-cantorlib.h )

add_library( cantorlibs  SHARED ${cantor_LIB_SRCS} )
generate_export_header(cantorlibs BASE_NAME cantor)

target_link_libraries( cantorlibs
  KF6::Completion
  KF6::IconThemes
  KF6::KIOCore
  KF6::KIOFileWidgets
  KF6::KIOWidgets
  KF6::Archive
  KF6::ConfigCore
  KF6::ConfigGui
  KF6::I18n
  KF6::XmlGui
  Qt6::Xml
  Qt6::Svg
  Poppler::Qt6
)

if(LIBSPECTRE_FOUND)
    target_link_libraries(cantorlibs ${LIBSPECTRE_LIBRARY})
endif(LIBSPECTRE_FOUND)

set (CANTORLIBS_SOVERSION 28)
set_target_properties( cantorlibs PROPERTIES VERSION ${RELEASE_SERVICE_VERSION} SOVERSION ${CANTORLIBS_SOVERSION})

ecm_setup_version(${RELEASE_SERVICE_VERSION}
    VARIABLE_PREFIX CANTOR
    SOVERSION ${CANTORLIBS_SOVERSION}
    VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/cantorlibs_version.h
)

install( TARGETS cantorlibs EXPORT CantorTargets  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

install(
  FILES
    ${cantor_LIB_HDRS}
    ${CMAKE_CURRENT_BINARY_DIR}/cantor_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/cantorlibs_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR}/cantor
  COMPONENT Devel
)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
