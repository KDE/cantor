set( SolveAssistant_SRCS 
     solveassistant.cpp
)

kde4_add_ui_files(SolveAssistant_SRCS solvedlg.ui)

kde4_add_plugin( cantor_solveassistant ${SolveAssistant_SRCS} )
target_link_libraries( cantor_solveassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES solveassistant.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install( FILES cantor_solve_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/cantor )
install(TARGETS cantor_solveassistant DESTINATION ${PLUGIN_INSTALL_DIR})
