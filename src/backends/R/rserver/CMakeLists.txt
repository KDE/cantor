include_directories(${R_INCLUDEDIR})
LINK_DIRECTORIES(${R_SHAREDLIBDIR})

set( RServer_SRCS 
   rserver.cpp
   rcallbacks.cpp
   main.cpp
)

kde4_add_kcfg_files(RServer_SRCS settings.kcfgc)
install(FILES rserver.kcfg DESTINATION ${KCFG_INSTALL_DIR})

QT4_ADD_DBUS_ADAPTOR(RServer_SRCS org.kde.Cantor.R.xml
                     rserver.h RServer)


add_custom_target(renvvars COMMAND ${R_EXECUTABLE} --slave --file=${CMAKE_CURRENT_SOURCE_DIR}/tools/envvars.r > ${CMAKE_CURRENT_BINARY_DIR}/renvvars.h )
add_custom_target(rautoloads COMMAND ${R_EXECUTABLE} --slave --file=${CMAKE_CURRENT_SOURCE_DIR}/tools/autoloads.r > ${CMAKE_CURRENT_BINARY_DIR}/rautoloads.h )

kde4_add_executable( cantor_rserver ${RServer_SRCS} )
target_link_libraries( cantor_rserver ${KDE4_KDEUI_LIBS} cantorlibs ${R_USED_LIBS} ${KDE4_KIO_LIBS})
add_dependencies(cantor_rserver renvvars rautoloads)

install(TARGETS cantor_rserver ${INSTALL_TARGETS_DEFAULT_ARGS} )
