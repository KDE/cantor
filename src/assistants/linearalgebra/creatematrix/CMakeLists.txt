set( CreateMatrixAssistant_SRCS 
     creatematrixassistant.cpp
     creatematrixdlg.cpp
)

kde4_add_ui_files(CreateMatrixAssistant_SRCS creatematrixdlg.ui)

kde4_add_plugin( cantor_creatematrixassistant ${CreateMatrixAssistant_SRCS} )
target_link_libraries( cantor_creatematrixassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES creatematrixassistant.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install( FILES cantor_create_matrix_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/cantor )
install(TARGETS cantor_creatematrixassistant DESTINATION ${PLUGIN_INSTALL_DIR})
