set( PythonBackend_SRCS
  pythonbackend.cpp
  pythonsession.cpp
  pythonexpression.cpp
  pythonkeywords.cpp
  pythonvariablemodel.cpp
  pythonhighlighter.cpp
  pythoncompletionobject.cpp
  pythonextensions.cpp
)

set(PythonServer_SRCS
  pythonservermain.cpp
  pythonserver.cpp
)

qt5_add_resources(PythonBackend_RSCS python.qrc)
ki18n_wrap_ui(PythonBackend_SRCS settings.ui)
kconfig_add_kcfg_files(PythonBackend_SRCS settings.kcfgc)

add_backend(pythonbackend ${PythonBackend_SRCS} ${PythonBackend_RSCS})
target_link_libraries(cantor_pythonbackend
  cantorlibs
  KF5::KIOCore
  KF5::ConfigCore
  KF5::ConfigGui
  KF5::SyntaxHighlighting
)

add_executable(cantor_pythonserver ${PythonServer_SRCS})
set_target_properties(cantor_pythonserver PROPERTIES INSTALL_RPATH_USE_LINK_PATH false)
if(MSVC)
  set_property(TARGET cantor_pythonserver PROPERTY LINK_FLAGS "/SUBSYSTEM:CONSOLE")
endif()
target_link_libraries(cantor_pythonserver Python3::Python)

if(BUILD_TESTING)
  add_executable(testpython testpython.cpp settings.cpp)
  add_test(NAME testpython COMMAND testpython)
  target_link_libraries(testpython
    Qt5::Test
    KF5::ConfigCore
    KF5::ConfigGui
    cantorlibs
    cantortest
  )
endif(BUILD_TESTING)

install(FILES cantor_python.knsrc  DESTINATION  ${KDE_INSTALL_KNSRCDIR})
install(FILES pythonbackend.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

install(TARGETS cantor_pythonbackend DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS cantor_pythonserver DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES graphic_packages.xml DESTINATION ${KDE_INSTALL_DATADIR}/cantor/python)

#install(DIRECTORY . DESTINATION ${KDE_INSTALL_DATADIR}/cantor/pythonbackend FILES_MATCHING PATTERN "*.py")
