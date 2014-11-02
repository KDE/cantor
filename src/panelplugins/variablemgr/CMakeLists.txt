set( VariableManagerPlugin_SRCS 
     variablemanagerplugin.cpp
     variablemanagerwidget.cpp
)

ki18n_wrap_ui(VariableManagerPlugin_SRCS newvardlg.ui)

add_library(cantor_variablemanagerplugin MODULE ${VariableManagerPlugin_SRCS} )
target_link_libraries( cantor_variablemanagerplugin ${KDE4_KDEUI_LIBS} KF5::KIOWidgets cantorlibs)

install( FILES variablemanagerplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
#install( FILES cantor_help_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/cantor )
install(TARGETS cantor_variablemanagerplugin DESTINATION ${PLUGIN_INSTALL_DIR})
