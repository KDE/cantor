set( VariableManagerPlugin_SRCS 
     variablemanagerplugin.cpp
     variablemanagerwidget.cpp
)

ki18n_wrap_ui(VariableManagerPlugin_SRCS newvardlg.ui)

add_panel(variablemanagerplugin ${VariableManagerPlugin_SRCS})

target_link_libraries(cantor_variablemanagerplugin ${KDE4_KDEUI_LIBS} KF5::KIOWidgets)
