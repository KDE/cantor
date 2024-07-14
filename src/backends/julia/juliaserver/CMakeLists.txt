add_definitions(-DJULIA_ENABLE_THREADING=1)

set(JuliaServer_SRCS
    juliaserver.cpp
    main.cpp
)
add_executable(cantor_juliaserver ${JuliaServer_SRCS})

target_link_libraries(cantor_juliaserver
    ${JULIA_LIBRARY}
    ${JULIA_INTERNAL_LIBRARY}
    Qt6::Widgets
    Qt6::DBus
)

install(TARGETS cantor_juliaserver ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
