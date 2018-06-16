set( Plot3dAssistant_SRCS
     plot3dassistant.cpp
)

ki18n_wrap_ui(Plot3dAssistant_SRCS plot3ddlg.ui)

add_assistant(plot3dassistant ${Plot3dAssistant_SRCS})

install( FILES cantor_plot3d_assistant.rc  DESTINATION  ${KXMLGUI_INSTALL_DIR}/cantor )
