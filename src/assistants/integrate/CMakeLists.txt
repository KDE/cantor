set( IntegrateAssistant_SRCS 
     integrateassistant.cpp
)

kde4_add_ui_files(IntegrateAssistant_SRCS integratedlg.ui)

kde4_add_plugin( cantor_integrateassistant ${IntegrateAssistant_SRCS} )
target_link_libraries( cantor_integrateassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES integrateassistant.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install( FILES cantor_integrate_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/cantor )
install(TARGETS cantor_integrateassistant DESTINATION ${PLUGIN_INSTALL_DIR})
