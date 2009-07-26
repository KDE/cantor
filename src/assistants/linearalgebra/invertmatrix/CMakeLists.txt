set( InvertMatrixAssistant_SRCS 
     invertmatrixassistant.cpp
)

kde4_add_ui_files(InvertMatrixAssistant_SRCS invertmatrixdlg.ui)

kde4_add_plugin( mathematik_invertmatrixassistant ${InvertMatrixAssistant_SRCS} )
target_link_libraries( mathematik_invertmatrixassistant ${KDE4_KDEUI_LIBS} mathematiklibs)

install( FILES invertmatrixassistant.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/mathematik)
install( FILES mathematik_invert_matrix_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/mathematik )
install(TARGETS mathematik_invertmatrixassistant DESTINATION ${PLUGIN_INSTALL_DIR})
