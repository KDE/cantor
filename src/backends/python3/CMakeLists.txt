set(Python3Backend_SRCS
  python3backend.cpp
  python3session.cpp
)

kconfig_add_kcfg_files(Python3Backend_SRCS settings.kcfgc)

add_backend(python3backend ${Python3Backend_SRCS})

target_link_libraries(cantor_python3backend
  cantor_pythonbackend
  Qt5::DBus)

add_subdirectory(python3server)

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
