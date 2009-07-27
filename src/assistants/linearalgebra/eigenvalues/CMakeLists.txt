set( EigenValuesAssistant_SRCS 
     eigenvaluesassistant.cpp
)

kde4_add_ui_files(EigenValuesAssistant_SRCS eigenvaluesdlg.ui)

kde4_add_plugin( mathematik_eigenvaluesassistant ${EigenValuesAssistant_SRCS} )
target_link_libraries( mathematik_eigenvaluesassistant ${KDE4_KDEUI_LIBS} mathematiklibs)

install( FILES eigenvaluesassistant.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/mathematik)
install( FILES mathematik_eigenvalues_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/mathematik )
install(TARGETS mathematik_eigenvaluesassistant DESTINATION ${PLUGIN_INSTALL_DIR})
