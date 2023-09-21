set( CreateMatrixAssistant_SRCS
     creatematrixassistant.cpp
     creatematrixdlg.cpp
     creatematrixassistant.qrc
)

ki18n_wrap_ui(CreateMatrixAssistant_SRCS creatematrixdlg.ui)

add_assistant(creatematrixassistant ${CreateMatrixAssistant_SRCS})
