set( PythonBackend_SRCS
  pythonbackend.cpp
  pythonsession.cpp
  pythonexpression.cpp
  pythonkeywords.cpp
  pythonvariablemodel.cpp
  pythonhighlighter.cpp
  pythoncompletionobject.cpp
  pythonextensions.cpp
  pythonsettingswidget.cpp
  ../backendsettingswidget.cpp
)

set(PythonServer_SRCS
  pythonservermain.cpp
  pythonserver.cpp
)

qt_add_resources(PythonBackend_RSCS python.qrc)
ki18n_wrap_ui(PythonBackend_SRCS settings.ui)
kconfig_add_kcfg_files(PythonBackend_SRCS settings.kcfgc)

add_backend(pythonbackend ${PythonBackend_SRCS} ${PythonBackend_RSCS})
target_link_libraries(cantor_pythonbackend
  cantorlibs
  KF6::KIOCore
  KF6::ConfigCore
  KF6::ConfigGui
  KF6::SyntaxHighlighting
)

if (ENABLE_EMBEDDED_DOCUMENTATION)
    target_link_libraries(cantor_pythonbackend cantor_help)
endif ()

add_executable(cantor_pythonserver ${PythonServer_SRCS})
set_target_properties(cantor_pythonserver PROPERTIES INSTALL_RPATH_USE_LINK_PATH false)
if(MSVC)
  set_property(TARGET cantor_pythonserver PROPERTY LINK_FLAGS "/SUBSYSTEM:CONSOLE")
endif()
target_include_directories(cantor_pythonserver PRIVATE ${Python3_INCLUDE_DIRS})
target_compile_definitions(cantor_pythonserver PRIVATE Py_LIMITED_API=0x03060000)
target_link_options(cantor_pythonserver PRIVATE "LINKER:--copy-dt-needed-entries")
target_link_libraries(cantor_pythonserver PRIVATE python3)

if(BUILD_TESTING)
  add_executable(testpython testpython.cpp settings.cpp)
  add_test(NAME testpython COMMAND testpython)
  target_link_libraries(testpython
    Qt6::Test
    KF6::ConfigCore
    KF6::ConfigGui
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
