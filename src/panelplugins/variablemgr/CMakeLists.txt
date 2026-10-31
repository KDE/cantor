set( VariableManagerPlugin_SRCS
     variablemanagerplugin.cpp
     variablemanagerwidget.cpp
)

ki18n_wrap_ui(VariableManagerPlugin_SRCS newvardlg.ui)

add_panel(variablemanagerplugin ${VariableManagerPlugin_SRCS})
