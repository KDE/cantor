set( SolveAssistant_SRCS
     solveassistant.cpp
)

ki18n_wrap_ui(SolveAssistant_SRCS solvedlg.ui)

add_library( cantor_solveassistant MODULE ${SolveAssistant_SRCS} )
target_link_libraries( cantor_solveassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES solveassistant.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/cantor)
install( FILES cantor_solve_assistant.rc  DESTINATION  ${KDE_INSTALL_DATADIR}/cantor )
install(TARGETS cantor_solveassistant DESTINATION ${KDE_INSTALL_PLUGINDIR})
