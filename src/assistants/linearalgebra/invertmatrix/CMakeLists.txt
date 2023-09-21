set( InvertMatrixAssistant_SRCS
     invertmatrixassistant.cpp
     invertmatrixassistant.qrc
)

ki18n_wrap_ui(InvertMatrixAssistant_SRCS invertmatrixdlg.ui)

add_assistant(invertmatrixassistant ${InvertMatrixAssistant_SRCS})
