function(add_panel name)

    kcoreaddons_add_plugin("cantor_${name}"
        SOURCES ${ARGN}
        INSTALL_NAMESPACE "cantor_panels")

    target_link_libraries("cantor_${name}"
    cantorlibs)

endfunction()

add_subdirectory(helppanel)
add_subdirectory(variablemgr)
add_subdirectory(filebrowserpanel)
add_subdirectory(tocpanel)
add_subdirectory(documentationpanel)
