set( Plot2dAssistant_SRCS
     plot2dassistant.cpp
)

ki18n_wrap_ui(Plot2dAssistant_SRCS plot2ddlg.ui)

add_assistant(plot2dassistant ${Plot2dAssistant_SRCS})

install( FILES cantor_plot2d_assistant.rc  DESTINATION  ${KDE_INSTALL_DATADIR}/cantor )
