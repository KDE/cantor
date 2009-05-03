set( SolveAssistant_SRCS 
     solveassistant.cpp
)

kde4_add_ui_files(SolveAssistant_SRCS solvedlg.ui)

kde4_add_plugin( mathematik_solveassistant ${SolveAssistant_SRCS} )
target_link_libraries( mathematik_solveassistant ${KDE4_KDEUI_LIBS} mathematiklibs)

install( FILES solveassistant.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/mathematik)
install( FILES mathematik_solve_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/mathematik )
install(TARGETS mathematik_solveassistant DESTINATION ${PLUGIN_INSTALL_DIR})
