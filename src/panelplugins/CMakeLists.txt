function(add_panel name)

    kcoreaddons_add_plugin("cantor_${name}"
        SOURCES ${ARGN}
        INSTALL_NAMESPACE "cantor_plugins/panels")

    target_link_libraries("cantor_${name}"
    cantorlibs)

endfunction()

add_subdirectory(helppanel)
add_subdirectory(variablemgr)
add_subdirectory(filebrowserpanel)
add_subdirectory(tocpanel)
if (ENABLE_EMBEDDED_DOCUMENTATION)
    add_subdirectory(documentationpanel)
endif ()
