set( Python3Backend_SRCS
     python3backend.cpp
     python3session.cpp
)

kconfig_add_kcfg_files(Python3Backend_SRCS settings.kcfgc)

add_backend(python3backend ${Python3Backend_SRCS})

target_link_libraries(cantor_python3backend
    cantor_pythonbackend
    Qt5::DBus)

install(FILES cantor_python3.knsrc  DESTINATION  ${KDE_INSTALL_CONFDIR})
install(FILES python3backend.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

add_subdirectory(python3server)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
