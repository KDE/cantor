set( DifferentiateAssistant_SRCS
     differentiateassistant.cpp
     differentiateassistant.qrc
)

ki18n_wrap_ui(DifferentiateAssistant_SRCS differentiatedlg.ui)

add_assistant(differentiateassistant ${DifferentiateAssistant_SRCS})
