include_directories(${R_INCLUDEDIR})
LINK_DIRECTORIES(${R_SHAREDLIBDIR})

if(NOT WIN32)
# rserver is using RInterface.h which is Unix only
# see https://colinfay.me/writing-r-extensions/linking-guis-and-other-front-ends-to-r.html
add_subdirectory(rserver)
endif()

set( RBackend_SRCS
  rbackend.cpp
  rsession.cpp
  rexpression.cpp
  rextensions.cpp
  rcompletionobject.cpp
  rhighlighter.cpp
  rkeywords.cpp
  rsettingswidget.cpp
  rvariablemodel.cpp
  ../backendsettingswidget.cpp
)

kconfig_add_kcfg_files(RBackend_SRCS rserver/settings.kcfgc)

set(network_xml rserver/org.kde.Cantor.R.xml)
qt_add_dbus_interface(RBackend_SRCS ${network_xml} rserver_interface )

ki18n_wrap_ui(RBackend_SRCS settings.ui)

add_backend(rbackend ${RBackend_SRCS})

set_target_properties( cantor_rbackend PROPERTIES INSTALL_RPATH_USE_LINK_PATH false)
target_link_libraries( cantor_rbackend cantor_help ${R_USED_LIBS} KF6::SyntaxHighlighting)
if(MSVC)
# When compiling with MSVC, we have to create a .lib file for R.dll, first
find_package(Python3 COMPONENTS Interpreter REQUIRED)
add_custom_target(R_lib COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/GenLibFile.py ${R_R_LIBRARY} ${CMAKE_BINARY_DIR} ${CMAKE_SYSTEM_PROCESSOR}
                        COMMENT "Generate .lib file for R.dll")
add_dependencies(cantor_rbackend R_lib)
endif(MSVC)

if(BUILD_TESTING)
  add_executable( testr testr.cpp)
  add_test(NAME testr COMMAND testr)
  ecm_mark_as_test(testr)
  target_link_libraries( testr
    Qt6::Test
    cantorlibs
    cantortest
  )
endif(BUILD_TESTING)

install( FILES cantor_r.knsrc  DESTINATION  ${KDE_INSTALL_KNSRCDIR} )
