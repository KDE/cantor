set( DifferentiateAssistant_SRCS 
     differentiateassistant.cpp
)

kde4_add_ui_files(DifferentiateAssistant_SRCS differentiatedlg.ui)

kde4_add_plugin( mathematik_differentiateassistant ${DifferentiateAssistant_SRCS} )
target_link_libraries( mathematik_differentiateassistant ${KDE4_KDEUI_LIBS} mathematiklibs)

install( FILES differentiateassistant.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/mathematik)
install( FILES mathematik_differentiate_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/mathematik )
install(TARGETS mathematik_differentiateassistant DESTINATION ${PLUGIN_INSTALL_DIR})
