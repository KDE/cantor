if (WITH_EPS)
set(PLOT_FILE_FORMAT epsc)
set(PLOT_FILE_SUFFIX eps)
else (WITH_EPS)
set(PLOT_FILE_FORMAT png)
set(PLOT_FILE_SUFFIX png)
endif (WITH_EPS)

configure_file(cantor_print.m.in cantor_print.m)
install( FILES cantor_eigenvectors.m cantor_plot2d.m cantor_plot3d.m ${CMAKE_CURRENT_BINARY_DIR}/cantor_print.m DESTINATION  ${OCTAVE_SCRIPT_INSTALL_DIR} )

