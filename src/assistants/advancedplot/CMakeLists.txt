set( AdvancedPlotAssistant_SRCS
     advancedplotassistant.cpp
)

ki18n_wrap_ui(AdvancedPlotAssistant_SRCS advancedplotdialog.ui directivecontainer.ui)

add_assistant(advancedplotassistant ${AdvancedPlotAssistant_SRCS})

install(FILES cantor_advancedplot_assistant.rc  DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/cantor)
