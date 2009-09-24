set( DifferentiateAssistant_SRCS 
     differentiateassistant.cpp
)

kde4_add_ui_files(DifferentiateAssistant_SRCS differentiatedlg.ui)

kde4_add_plugin( cantor_differentiateassistant ${DifferentiateAssistant_SRCS} )
target_link_libraries( cantor_differentiateassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES differentiateassistant.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install( FILES cantor_differentiate_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/cantor )
install(TARGETS cantor_differentiateassistant DESTINATION ${PLUGIN_INSTALL_DIR})
