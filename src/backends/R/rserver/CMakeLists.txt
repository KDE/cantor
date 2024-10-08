include_directories(${R_INCLUDE_DIR})

set( RServer_SRCS
   rserver.cpp
   rcallbacks.cpp
   main.cpp
)

kconfig_add_kcfg_files(RServer_SRCS settings.kcfgc)
install(FILES rserver.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

qt_add_dbus_adaptor(RServer_SRCS org.kde.Cantor.R.xml rserver.h RServer)


add_custom_command(OUTPUT renvvars.h COMMAND ${R_EXECUTABLE} --slave --file=${CMAKE_CURRENT_SOURCE_DIR}/tools/envvars.r > ${CMAKE_CURRENT_BINARY_DIR}/renvvars.h)
add_custom_target(renvvars DEPENDS renvvars.h)

add_custom_command(OUTPUT rautoloads.h COMMAND ${R_EXECUTABLE} --slave --file=${CMAKE_CURRENT_SOURCE_DIR}/tools/autoloads.r > ${CMAKE_CURRENT_BINARY_DIR}/rautoloads.h )
add_custom_target(rautoloads DEPENDS rautoloads.h)

add_executable( cantor_rserver ${RServer_SRCS} )
set_target_properties( cantor_rserver PROPERTIES INSTALL_RPATH_USE_LINK_PATH false)
target_link_libraries( cantor_rserver cantorlibs ${R_LIBRARIES} KF6::KIOCore)
add_dependencies(cantor_rserver renvvars rautoloads)

install(TARGETS cantor_rserver ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
