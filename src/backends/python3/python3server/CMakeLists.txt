include_directories(${PYTHONLIBS3_INCLUDE_DIRS})

set(Python3Server_SRCS
    main.cpp
    ../../python/pythonserver.cpp
)

add_executable(cantor_python3server ${Python3Server_SRCS})

set_target_properties(cantor_python3server PROPERTIES INSTALL_RPATH_USE_LINK_PATH false)
target_link_libraries(cantor_python3server
    ${PYTHONLIBS3_LIBRARIES}
    Qt5::Widgets
    Qt5::DBus)

install(TARGETS cantor_python3server ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
