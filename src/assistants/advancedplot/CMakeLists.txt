set( AdvancedPlotAssistant_SRCS
     advancedplotassistant.cpp
)

ki18n_wrap_ui(AdvancedPlotAssistant_SRCS advancedplotdialog.ui directivecontainer.ui)

add_library( cantor_advancedplotassistant MODULE ${AdvancedPlotAssistant_SRCS} )
target_link_libraries( cantor_advancedplotassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES advancedplotassistant.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/cantor)
install( FILES cantor_advancedplot_assistant.rc  DESTINATION  ${KDE_INSTALL_DATADIR}/cantor )
install(TARGETS cantor_advancedplotassistant DESTINATION ${KDE_INSTALL_PLUGINDIR})
