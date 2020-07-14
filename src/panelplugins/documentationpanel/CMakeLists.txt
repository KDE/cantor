set( DocumentationPanelPlugin_SRCS
     documentationpanelplugin.cpp
     documentationpanelwidget.cpp
)

add_panel(documentationpanelplugin ${DocumentationPanelPlugin_SRCS})

target_link_libraries(cantor_documentationpanelplugin
    KF5::CoreAddons
    Qt5::Widgets
    Qt5::Help
    Qt5::Gui
    Qt5::Core
    Qt5::WebKitWidgets
    Qt5::WebEngine
    Qt5::WebEngineWidgets)
