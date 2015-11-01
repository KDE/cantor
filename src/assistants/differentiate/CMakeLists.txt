set( DifferentiateAssistant_SRCS
     differentiateassistant.cpp
)

ki18n_wrap_ui(DifferentiateAssistant_SRCS differentiatedlg.ui)

add_assistant(differentiateassistant ${DifferentiateAssistant_SRCS})

install( FILES cantor_differentiate_assistant.rc  DESTINATION  ${KDE_INSTALL_DATADIR}/cantor )
