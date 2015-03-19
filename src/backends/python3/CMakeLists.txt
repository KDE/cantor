set( Python3Backend_SRCS
     python3backend.cpp
     python3session.cpp
)

kconfig_add_kcfg_files(Python3Backend_SRCS settings.kcfgc)

add_library(cantor_python3backend MODULE ${Python3Backend_SRCS})
target_link_libraries(cantor_python3backend
    cantorlibs
    cantor_pythonbackend
    Qt5::DBus)

install(FILES cantor_python3.knsrc  DESTINATION  ${KDE_INSTALL_CONFDIR})
install(FILES python3backend.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/cantor)
install(TARGETS cantor_python3backend DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES python3backend.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

add_subdirectory(python3server)
add_subdirectory(tests)
