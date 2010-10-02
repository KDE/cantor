include_directories(${R_INCLUDEDIR})
LINK_DIRECTORIES(${R_SHAREDLIBDIR})

add_subdirectory(rserver)

set( RBackend_SRCS
 rbackend.cpp
 rsession.cpp
 rexpression.cpp
 rextensions.cpp
 rcompletionobject.cpp
 rhighlighter.cpp
 rsettingswidget.cpp
)

kde4_add_kcfg_files(RBackend_SRCS rserver/settings.kcfgc)

set(network_xml rserver/org.kde.Cantor.R.xml)
QT4_ADD_DBUS_INTERFACE(RBackend_SRCS ${network_xml} rserver_interface )

kde4_add_ui_files(RBackend_SRCS settings.ui)

kde4_add_plugin( cantor_rbackend ${RBackend_SRCS} )
target_link_libraries( cantor_rbackend ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} cantorlibs ${R_USED_LIBS})

install( FILES rbackend.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install(TARGETS cantor_rbackend DESTINATION ${PLUGIN_INSTALL_DIR})
