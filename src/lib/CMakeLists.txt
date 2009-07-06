set(GENERIC_LIB_VERSION "0.0.1")
set(GENERIC_LIB_SOVERSION "1")

set( mathematik_LIB_SRCS
  session.cpp
  expression.cpp
  backend.cpp
  result.cpp
  textresult.cpp
  imageresult.cpp
  epsresult.cpp
  helpresult.cpp
  extension.cpp
  assistant.cpp
  tabcompletionobject.cpp
)
 
set( mathematik_LIB_HDRS 
     backend.h
     session.h
     expression.h
     backend_macros.h
)

kde4_add_library( mathematiklibs  SHARED ${mathematik_LIB_SRCS} )
 
target_link_libraries( mathematiklibs
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KIO_LIBS}
  ${LIBSPECTRE_LIBRARY}
)
set_target_properties( mathematiklibs
  PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION}
)
 
install( TARGETS mathematiklibs  ${INSTALL_TARGETS_DEFAULT_ARGS} )

#install( FILES ${myshared_LIB_HDRS}
#  DESTINATION ${INCLUDE_INSTALL_DIR}/mathematik
#  COMPONENT Devel
#)

install( FILES mathematik_backend.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})
install( FILES mathematik_assistant.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})

