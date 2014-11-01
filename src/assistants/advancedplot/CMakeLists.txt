set( AdvancedPlotAssistant_SRCS
     advancedplotassistant.cpp
)

ki18n_wrap_ui(AdvancedPlotAssistant_SRCS advancedplotdialog.ui directivecontainer.ui)

add_library( cantor_advancedplotassistant MODULE ${AdvancedPlotAssistant_SRCS} )
target_link_libraries( cantor_advancedplotassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES advancedplotassistant.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install( FILES cantor_advancedplot_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/cantor )
install(TARGETS cantor_advancedplotassistant DESTINATION ${PLUGIN_INSTALL_DIR})
