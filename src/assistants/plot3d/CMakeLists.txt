set( Plot3dAssistant_SRCS
     plot3dassistant.cpp
     plot3dassistant.qrc
)

ki18n_wrap_ui(Plot3dAssistant_SRCS plot3ddlg.ui)

add_assistant(plot3dassistant ${Plot3dAssistant_SRCS})
