set( InvertMatrixAssistant_SRCS
     invertmatrixassistant.cpp
)

ki18n_wrap_ui(InvertMatrixAssistant_SRCS invertmatrixdlg.ui)

add_assistant(invertmatrixassistant ${InvertMatrixAssistant_SRCS})

install( FILES cantor_invert_matrix_assistant.rc  DESTINATION  ${KXMLGUI_INSTALL_DIR}/cantor )
