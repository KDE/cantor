set( DocumentationPanelPlugin_SRCS
     documentationpanelplugin.cpp
     documentationpanelwidget.cpp
)

add_panel(documentationpanelplugin ${DocumentationPanelPlugin_SRCS})

target_link_libraries(cantor_documentationpanelplugin
    Qt6::Help
    Qt6::WebEngineWidgets)
