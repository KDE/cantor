set( SolveAssistant_SRCS
     solveassistant.cpp
)

ki18n_wrap_ui(SolveAssistant_SRCS solvedlg.ui)

add_library( cantor_solveassistant MODULE ${SolveAssistant_SRCS} )
target_link_libraries( cantor_solveassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES solveassistant.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install( FILES cantor_solve_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/cantor )
install(TARGETS cantor_solveassistant DESTINATION ${PLUGIN_INSTALL_DIR})
