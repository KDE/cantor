set(GENERIC_LIB_VERSION "0.0.1")
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
  extension.cpp
  assistant.cpp
  tabcompletionobject.cpp
  defaulthighlighter.cpp
)
 
Set( cantor_LIB_HDRS 
     backend.h
     session.h
     expression.h
     backend_macros.h
)

kde4_add_kcfg_files(cantor_LIB_SRCS settings.kcfgc)
install(FILES cantor_libs.kcfg DESTINATION ${KCFG_INSTALL_DIR})

configure_file (config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

kde4_add_library( cantorlibs  SHARED ${cantor_LIB_SRCS} )
 
target_link_libraries( cantorlibs
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KIO_LIBS}
)
set_target_properties( cantorlibs
  PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION}
)
 
install( TARGETS cantorlibs  ${INSTALL_TARGETS_DEFAULT_ARGS} )

#install( FILES ${myshared_LIB_HDRS}
#  DESTINATION ${INCLUDE_INSTALL_DIR}/cantor
#  COMPONENT Devel
#)

install( FILES cantor_backend.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})
install( FILES cantor_assistant.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})
