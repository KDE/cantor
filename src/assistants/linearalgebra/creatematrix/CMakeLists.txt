set( CreateMatrixAssistant_SRCS
     creatematrixassistant.cpp
     creatematrixdlg.cpp
)

ki18n_wrap_ui(CreateMatrixAssistant_SRCS creatematrixdlg.ui)

add_assistant(creatematrixassistant ${CreateMatrixAssistant_SRCS})

install( FILES cantor_create_matrix_assistant.rc  DESTINATION  ${KXMLGUI_INSTALL_DIR}/cantor )
