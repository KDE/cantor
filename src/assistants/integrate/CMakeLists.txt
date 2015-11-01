set( IntegrateAssistant_SRCS
     integrateassistant.cpp
)

ki18n_wrap_ui(IntegrateAssistant_SRCS integratedlg.ui)

add_assistant(integrateassistant ${IntegrateAssistant_SRCS})

install( FILES cantor_integrate_assistant.rc  DESTINATION  ${KDE_INSTALL_DATADIR}/cantor )
