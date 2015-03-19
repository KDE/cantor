set( HelpPanelPlugin_SRCS
     helppanelplugin.cpp
)


add_library(cantor_helppanelplugin MODULE ${HelpPanelPlugin_SRCS} )
target_link_libraries( cantor_helppanelplugin cantorlibs)

install( FILES helppanelplugin.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/cantor)
#install( FILES cantor_help_assistant.rc  DESTINATION  ${KDE_INSTALL_DATADIR}/cantor )
install(TARGETS cantor_helppanelplugin DESTINATION ${KDE_INSTALL_PLUGINDIR})
