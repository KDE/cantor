include_directories(${PYTHON_INCLUDE_DIR})

set(Python2Server_SRCS
    main.cpp
    ../../python/pythonserver.cpp
)

add_executable(cantor_python2server ${Python2Server_SRCS})

set_target_properties(cantor_python2server PROPERTIES INSTALL_RPATH_USE_LINK_PATH false)
target_link_libraries(cantor_python2server
    ${PYTHON_LIBRARIES}
    Qt5::Widgets
    Qt5::DBus)

install(TARGETS cantor_python2server ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
