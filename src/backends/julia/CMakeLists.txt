include_directories(${JULIA_INCLUDE_DIRS})

add_subdirectory(juliaserver)

set(JuliaBackend_SRCS
  juliabackend.cpp
  juliasession.cpp
  juliaexpression.cpp
  juliakeywords.cpp
  juliahighlighter.cpp
  juliaextensions.cpp
  juliacompletionobject.cpp
)

kconfig_add_kcfg_files(JuliaBackend_SRCS settings.kcfgc)
ki18n_wrap_ui(JuliaBackend_SRCS settings.ui)

add_backend(juliabackend ${JuliaBackend_SRCS})

target_link_libraries(cantor_juliabackend
  ${JULIA_LIBRARY}
  Qt5::DBus
  KF5::SyntaxHighlighting
)

if(BUILD_TESTING)
  add_executable(testjulia testjulia.cpp)
  add_test(NAME testjulia COMMAND testjulia)
  target_link_libraries(testjulia 
    Qt5::Test
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
