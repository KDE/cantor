set( InvertMatrixAssistant_SRCS 
     invertmatrixassistant.cpp
)

kde4_add_ui_files(InvertMatrixAssistant_SRCS invertmatrixdlg.ui)

kde4_add_plugin( cantor_invertmatrixassistant ${InvertMatrixAssistant_SRCS} )
target_link_libraries( cantor_invertmatrixassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES invertmatrixassistant.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install( FILES cantor_invert_matrix_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/cantor )
install(TARGETS cantor_invertmatrixassistant DESTINATION ${PLUGIN_INSTALL_DIR})
