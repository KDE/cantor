set( EigenVectorsAssistant_SRCS 
     eigenvectorsassistant.cpp
)

kde4_add_ui_files(EigenVectorsAssistant_SRCS eigenvectorsdlg.ui)

kde4_add_plugin( cantor_eigenvectorsassistant ${EigenVectorsAssistant_SRCS} )
target_link_libraries( cantor_eigenvectorsassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES eigenvectorsassistant.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install( FILES cantor_eigenvectors_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/cantor )
install(TARGETS cantor_eigenvectorsassistant DESTINATION ${PLUGIN_INSTALL_DIR})
