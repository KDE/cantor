function(add_assistant name)

    kcoreaddons_add_plugin("cantor_${name}"
        SOURCES ${ARGN}
        INSTALL_NAMESPACE "cantor/assistants")

    target_link_libraries("cantor_${name}"
        cantorlibs)

endfunction()


add_subdirectory(solve)
add_subdirectory(integrate)
add_subdirectory(differentiate)
add_subdirectory(linearalgebra)
add_subdirectory(runscript)
add_subdirectory(plot2d)
add_subdirectory(plot3d)
add_subdirectory(advancedplot)
add_subdirectory(importpackage)
