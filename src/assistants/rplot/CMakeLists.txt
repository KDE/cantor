set( RPlotAssistant_SRCS
     rplotassistant.cpp
)

kde4_add_ui_files(RPlotAssistant_SRCS rplotdialog.ui)

kde4_add_plugin( cantor_rplotassistant ${RPlotAssistant_SRCS} )
target_link_libraries( cantor_rplotassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES rplotassistant.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install( FILES cantor_rplot_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/cantor )
install(TARGETS cantor_rplotassistant DESTINATION ${PLUGIN_INSTALL_DIR})
