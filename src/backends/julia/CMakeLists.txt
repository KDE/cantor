add_subdirectory(juliaserver)
add_subdirectory(tests)

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

target_link_libraries(cantor_juliabackend Qt5::DBus)

install(FILES juliabackend.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES keywords.xml DESTINATION ${KDE_INSTALL_DATADIR}/cantor/juliabackend)
file(GLOB scripts "${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.jl")
install(
    FILES ${scripts}
    DESTINATION ${KDE_INSTALL_DATADIR}/cantor/juliabackend/scripts
)
