set(Python3Backend_SRCS
  python3backend.cpp
  python3session.cpp
)

set(Python3Server_SRCS
  ../python/pythonservermain.cpp
  ../python/pythonserver.cpp
)

include_directories(${PYTHONLIBS3_INCLUDE_DIRS})
kconfig_add_kcfg_files(Python3Backend_SRCS settings.kcfgc)
add_backend(python3backend ${Python3Backend_SRCS})
target_link_libraries(cantor_python3backend cantor_pythonbackend)

add_executable(cantor_python3server ${Python3Server_SRCS})
set_target_properties(cantor_python3server PROPERTIES INSTALL_RPATH_USE_LINK_PATH false)
target_link_libraries(cantor_python3server ${PYTHONLIBS3_LIBRARIES})

if(BUILD_TESTING)
  add_executable(testpython3 testpython3.cpp settings.cpp)
  add_test(NAME testpython3 COMMAND testpython3)
  target_link_libraries(testpython3
    Qt5::Test
    KF5::ConfigCore
    KF5::ConfigGui
    cantorlibs
    cantortest
  )
endif(BUILD_TESTING)

install(FILES cantor_python3.knsrc  DESTINATION  ${KDE_INSTALL_CONFDIR})
install(FILES python3backend.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

install(TARGETS cantor_python3server ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
