set( CreateMatrixAssistant_SRCS
     creatematrixassistant.cpp
     creatematrixdlg.cpp
)

ki18n_wrap_ui(CreateMatrixAssistant_SRCS creatematrixdlg.ui)

add_library( cantor_creatematrixassistant MODULE ${CreateMatrixAssistant_SRCS} )
target_link_libraries( cantor_creatematrixassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES creatematrixassistant.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install( FILES cantor_create_matrix_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/cantor )
install(TARGETS cantor_creatematrixassistant DESTINATION ${PLUGIN_INSTALL_DIR})
