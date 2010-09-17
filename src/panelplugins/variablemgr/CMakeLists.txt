set( VariableManagerPlugin_SRCS 
     variablemanagerplugin.cpp
     variablemanagerwidget.cpp
)

kde4_add_ui_files(VariableManagerPlugin_SRCS newvardlg.ui)

kde4_add_plugin( cantor_variablemanagerplugin ${VariableManagerPlugin_SRCS} )
target_link_libraries( cantor_variablemanagerplugin ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} cantorlibs)

install( FILES variablemanagerplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
#install( FILES cantor_help_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/cantor )
install(TARGETS cantor_variablemanagerplugin DESTINATION ${PLUGIN_INSTALL_DIR})
