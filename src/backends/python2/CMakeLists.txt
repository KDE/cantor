set( Python2Backend_SRCS
     python2backend.cpp
     python2session.cpp
)

kconfig_add_kcfg_files(Python2Backend_SRCS settings.kcfgc)

if(MSVC)
    # ssize_t is typedef'd in both kdewin and python headers, this prevents using the kdewin one
    add_definitions(-D_SSIZE_T_DEFINED)
endif(MSVC)

include_directories(${PYTHON_LIBRARIES_DIR})
include_directories(${PYTHON_INCLUDE_DIR})

add_backend(python2backend ${Python2Backend_SRCS})

target_link_libraries(cantor_python2backend
    ${PYTHON_LIBRARIES}
    cantor_pythonbackend
    )

if(BUILD_TESTING)
  add_executable(testpython2 testpython2.cpp)
  target_link_libraries(testpython2 ${QT_QTTEST_LIBRARY} cantorlibs cantortest)

  add_test(NAME testpython2 COMMAND testpython2)
endif()

install(FILES cantor_python2.knsrc  DESTINATION  ${KDE_INSTALL_CONFDIR})
install(FILES python2backend.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
