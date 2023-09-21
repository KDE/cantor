set( AdvancedPlotAssistant_SRCS
     advancedplotassistant.cpp
     advancedplotassistant.qrc
)

ki18n_wrap_ui(AdvancedPlotAssistant_SRCS advancedplotdialog.ui directivecontainer.ui)

add_assistant(advancedplotassistant ${AdvancedPlotAssistant_SRCS})
