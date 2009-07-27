set( EigenVectorsAssistant_SRCS 
     eigenvectorsassistant.cpp
)

kde4_add_ui_files(EigenVectorsAssistant_SRCS eigenvectorsdlg.ui)

kde4_add_plugin( mathematik_eigenvectorsassistant ${EigenVectorsAssistant_SRCS} )
target_link_libraries( mathematik_eigenvectorsassistant ${KDE4_KDEUI_LIBS} mathematiklibs)

install( FILES eigenvectorsassistant.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/mathematik)
install( FILES mathematik_eigenvectors_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/mathematik )
install(TARGETS mathematik_eigenvectorsassistant DESTINATION ${PLUGIN_INSTALL_DIR})
