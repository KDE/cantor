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
)

ki18n_wrap_ui(cantor_LIB_SRCS directives/axisrange.ui directives/plottitle.ui)

kconfig_add_kcfg_files(cantor_LIB_SRCS settings.kcfgc)
install(FILES cantor_libs.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

configure_file (config-cantorlib.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-cantorlib.h )

add_library( cantorlibs  SHARED ${cantor_LIB_SRCS} )
generate_export_header(cantorlibs BASE_NAME cantor)

kcoreaddons_desktop_to_json(cantorlibs cantor_assistant.desktop DEFAULT_SERVICE_TYPE)
kcoreaddons_desktop_to_json(cantorlibs cantor_backend.desktop DEFAULT_SERVICE_TYPE)
kcoreaddons_desktop_to_json(cantorlibs cantor_panelplugin.desktop DEFAULT_SERVICE_TYPE)

target_link_libraries( cantorlibs
  KF5::Completion
  KF5::IconThemes
  KF5::KIOCore
  KF5::KIOFileWidgets
  KF5::KIOWidgets
  KF5::Archive
  KF5::ConfigCore
  KF5::ConfigGui
  KF5::I18n
  KF5::XmlGui
  ${QT5_LIBRARIES}
  Qt5::Xml
  Poppler::Qt5
)

if(LIBSPECTRE_FOUND)
    target_link_libraries(cantorlibs ${LIBSPECTRE_LIBRARY})
endif(LIBSPECTRE_FOUND)

set (CANTORLIBS_SOVERSION 24)
set_target_properties( cantorlibs PROPERTIES VERSION ${KDE_APPLICATIONS_VERSION} SOVERSION ${CANTORLIBS_SOVERSION})

ecm_setup_version(${KDE_APPLICATIONS_VERSION}
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
