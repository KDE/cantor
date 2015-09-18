set( HelpPanelPlugin_SRCS
     helppanelplugin.cpp
)

add_panel(helppanelplugin ${HelpPanelPlugin_SRCS})

target_link_libraries(cantor_helppanelplugin
    KF5::TextWidgets)
