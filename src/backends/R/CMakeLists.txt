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
 rkeywords.cpp
)

kconfig_add_kcfg_files(RBackend_SRCS settings.kcfgc)
install(FILES rbackend.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

#set(network_xml rserver/org.kde.Cantor.R.xml)
#QT5_ADD_DBUS_INTERFACE(RBackend_SRCS ${network_xml} rserver_interface )

ki18n_wrap_ui(RBackend_SRCS settings.ui)

add_backend(rbackend ${RBackend_SRCS})

set_target_properties( cantor_rbackend PROPERTIES INSTALL_RPATH_USE_LINK_PATH false)
target_link_libraries( cantor_rbackend ${R_USED_LIBS})

install( FILES cantor_r.knsrc  DESTINATION  ${KDE_INSTALL_CONFDIR} )
