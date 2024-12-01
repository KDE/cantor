include_directories(${JULIA_INCLUDE_DIRS})

find_package(Qt6 REQUIRED COMPONENTS DBus)
add_subdirectory(juliaserver)

set(JuliaBackend_SRCS
  juliabackend.cpp
  juliasession.cpp
  juliaexpression.cpp
  juliakeywords.cpp
  juliavariablemodel.cpp
  juliahighlighter.cpp
  juliaextensions.cpp
  juliacompletionobject.cpp
  juliasettingswidget.cpp
  ../backendsettingswidget.cpp
)

kconfig_add_kcfg_files(JuliaBackend_SRCS settings.kcfgc)
ki18n_wrap_ui(JuliaBackend_SRCS settings.ui)

add_backend(juliabackend ${JuliaBackend_SRCS})

target_link_libraries(cantor_juliabackend
  ${JULIA_LIBRARY}
  Qt6::DBus
  KF6::SyntaxHighlighting
)

if (ENABLE_EMBEDDED_DOCUMENTATION)
    target_link_libraries(cantor_juliabackend cantor_help)
endif ()

if(BUILD_TESTING)
  add_executable(testjulia testjulia.cpp settings.cpp)
  add_test(NAME testjulia COMMAND testjulia)
  target_link_libraries(testjulia
    Qt6::Test
    cantorlibs
    cantortest
  )
endif(BUILD_TESTING)

install(FILES juliabackend.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
file(GLOB scripts "${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.jl")
install(
  FILES ${scripts}
  DESTINATION ${KDE_INSTALL_DATADIR}/cantor/juliabackend/scripts
)

install(FILES graphic_packages.xml DESTINATION ${KDE_INSTALL_DATADIR}/cantor/julia)
