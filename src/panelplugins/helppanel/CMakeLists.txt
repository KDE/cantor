set( HelpPanelPlugin_SRCS 
     helppanelplugin.cpp
)


kde4_add_plugin( cantor_helppanelplugin ${HelpPanelPlugin_SRCS} )
target_link_libraries( cantor_helppanelplugin ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES helppanelplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
#install( FILES cantor_help_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/cantor )
install(TARGETS cantor_helppanelplugin DESTINATION ${PLUGIN_INSTALL_DIR})
