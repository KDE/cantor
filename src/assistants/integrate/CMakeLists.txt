set( IntegrateAssistant_SRCS 
     integrateassistant.cpp
)

kde4_add_ui_files(IntegrateAssistant_SRCS integratedlg.ui)

kde4_add_plugin( mathematik_integrateassistant ${IntegrateAssistant_SRCS} )
target_link_libraries( mathematik_integrateassistant ${KDE4_KDEUI_LIBS} mathematiklibs)

install( FILES integrateassistant.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/mathematik)
install( FILES mathematik_integrate_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/mathematik )
install(TARGETS mathematik_integrateassistant DESTINATION ${PLUGIN_INSTALL_DIR})
