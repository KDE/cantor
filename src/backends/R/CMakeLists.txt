include_directories(${R_INCLUDEDIR})
LINK_DIRECTORIES(${R_SHAREDLIBDIR})

#add_subdirectory(rserver)

set( RBackend_SRCS
 rbackend.cpp
 rsession.cpp
 rexpression.cpp
 rextensions.cpp
 rcompletionobject.cpp
 rhighlighter.cpp
 rsettingswidget.cpp
)

kconfig_add_kcfg_files(RBackend_SRCS rserver/settings.kcfgc)

set(network_xml rserver/org.kde.Cantor.R.xml)
QT5_ADD_DBUS_INTERFACE(RBackend_SRCS ${network_xml} rserver_interface )

ki18n_wrap_ui(RBackend_SRCS settings.ui)

add_library( cantor_rbackend MODULE ${RBackend_SRCS} )
set_target_properties( cantor_rbackend PROPERTIES INSTALL_RPATH_USE_LINK_PATH false)
target_link_libraries( cantor_rbackend cantorlibs ${R_USED_LIBS})

install( FILES rbackend.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install(TARGETS cantor_rbackend DESTINATION ${PLUGIN_INSTALL_DIR})
