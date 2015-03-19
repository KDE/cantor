set( VariableManagerPlugin_SRCS 
     variablemanagerplugin.cpp
     variablemanagerwidget.cpp
)

ki18n_wrap_ui(VariableManagerPlugin_SRCS newvardlg.ui)

add_library(cantor_variablemanagerplugin MODULE ${VariableManagerPlugin_SRCS} )
target_link_libraries( cantor_variablemanagerplugin ${KDE4_KDEUI_LIBS} KF5::KIOWidgets cantorlibs)

install( FILES variablemanagerplugin.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/cantor)
#install( FILES cantor_help_assistant.rc  DESTINATION  ${KDE_INSTALL_DATADIR}/cantor )
install(TARGETS cantor_variablemanagerplugin DESTINATION ${KDE_INSTALL_PLUGINDIR})
