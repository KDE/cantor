set(JuliaServer_SRCS
    juliaserver.cpp
    main.cpp
)

add_executable(cantor_juliaserver ${JuliaServer_SRCS})

target_link_libraries(cantor_juliaserver
    julia
    Qt5::Widgets
    Qt5::DBus
)

install(TARGETS cantor_juliaserver ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
