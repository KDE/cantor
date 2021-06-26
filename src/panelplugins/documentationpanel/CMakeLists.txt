set( DocumentationPanelPlugin_SRCS
     documentationpanelplugin.cpp
     documentationpanelwidget.cpp
)

add_panel(documentationpanelplugin ${DocumentationPanelPlugin_SRCS})

target_link_libraries(cantor_documentationpanelplugin
    Qt5::Help
    Qt5::WebEngine
    Qt5::WebEngineWidgets)
