function(add_panel name)

    kcoreaddons_add_plugin("cantor_${name}"
        SOURCES ${ARGN}
        JSON "${name}.json"
        INSTALL_NAMESPACE "cantor/panels")

    target_link_libraries("cantor_${name}"
    cantorpart
    cantorlibs)

endfunction()

add_subdirectory(helppanel)
add_subdirectory(variablemgr)
add_subdirectory(documentationpanel)
