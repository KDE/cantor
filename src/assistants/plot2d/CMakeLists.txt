set( Plot2dAssistant_SRCS
     plot2dassistant.cpp
     plot2dassistant.qrc
)

ki18n_wrap_ui(Plot2dAssistant_SRCS plot2ddlg.ui)

add_assistant(plot2dassistant ${Plot2dAssistant_SRCS})
