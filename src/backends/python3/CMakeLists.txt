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

install(FILES cantor_python3.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR})
install(FILES python3backend.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install(TARGETS cantor_python3backend DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES python3backend.kcfg DESTINATION ${KCFG_INSTALL_DIR})

add_subdirectory(python3server)
add_subdirectory(tests)