set(GENERIC_LIB_VERSION "0.0.2")
set(GENERIC_LIB_SOVERSION "1")

set( cantor_LIB_SRCS
  session.cpp
  expression.cpp
  backend.cpp
  result.cpp
  textresult.cpp
  imageresult.cpp
  epsresult.cpp
  latexresult.cpp
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
)

Set( cantor_LIB_HDRS
     backend.h
     session.h
     expression.h
     backend_macros.h
)

kde4_add_kcfg_files(cantor_LIB_SRCS settings.kcfgc)
install(FILES cantor_libs.kcfg DESTINATION ${KCFG_INSTALL_DIR})

configure_file (config-cantorlib.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-cantorlib.h )

kde4_add_library( cantorlibs  SHARED ${cantor_LIB_SRCS} )

target_link_libraries( cantorlibs
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KIO_LIBS}
)
set_target_properties( cantorlibs
  PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION}
)

if (KDE4_BUILD_TESTS)
    add_subdirectory(test)
endif (KDE4_BUILD_TESTS)

install( TARGETS cantorlibs  ${INSTALL_TARGETS_DEFAULT_ARGS} )

#install( FILES ${myshared_LIB_HDRS}
#  DESTINATION ${INCLUDE_INSTALL_DIR}/cantor
#  COMPONENT Devel
#)

install( FILES cantor_backend.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})
install( FILES cantor_assistant.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})
install( FILES cantor_panelplugin.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})
