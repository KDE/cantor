set( AdvancedPlotAssistant_SRCS
     advancedplotassistant.cpp
)

kde4_add_ui_files(AdvancedPlotAssistant_SRCS advancedplotdialog.ui directivecontainer.ui)

kde4_add_plugin( cantor_advancedplotassistant ${AdvancedPlotAssistant_SRCS} )
target_link_libraries( cantor_advancedplotassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES advancedplotassistant.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install( FILES cantor_advancedplot_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/cantor )
install(TARGETS cantor_advancedplotassistant DESTINATION ${PLUGIN_INSTALL_DIR})
