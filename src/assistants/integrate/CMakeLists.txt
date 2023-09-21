set( IntegrateAssistant_SRCS
     integrateassistant.cpp
     integrateassistant.qrc
)

ki18n_wrap_ui(IntegrateAssistant_SRCS integratedlg.ui)

add_assistant(integrateassistant ${IntegrateAssistant_SRCS})
