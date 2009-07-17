include_directories(${R_INCLUDEDIR})
LINK_DIRECTORIES(${R_SHAREDLIBDIR})

set( RServer_SRCS 
   rserver.cpp
   rcallbacks.cpp
   main.cpp
)

QT4_ADD_DBUS_ADAPTOR(RServer_SRCS org.MathematiK.R.xml
                     rserver.h RServer)

kde4_add_executable( mathematik_rserver ${RServer_SRCS} )
target_link_libraries( mathematik_rserver ${KDE4_KDEUI_LIBS} mathematiklibs ${R_USED_LIBS})

install(TARGETS mathematik_rserver ${INSTALL_TARGETS_DEFAULT_ARGS} )
