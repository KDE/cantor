set( CreateMatrixAssistant_SRCS 
     creatematrixassistant.cpp
     creatematrixdlg.cpp
)

kde4_add_ui_files(CreateMatrixAssistant_SRCS creatematrixdlg.ui)

kde4_add_plugin( mathematik_creatematrixassistant ${CreateMatrixAssistant_SRCS} )
target_link_libraries( mathematik_creatematrixassistant ${KDE4_KDEUI_LIBS} mathematiklibs)

install( FILES creatematrixassistant.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/mathematik)
install( FILES mathematik_create_matrix_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/mathematik )
install(TARGETS mathematik_creatematrixassistant DESTINATION ${PLUGIN_INSTALL_DIR})
