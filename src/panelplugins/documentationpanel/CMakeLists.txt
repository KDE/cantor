set( DocumentationPanelPlugin_SRCS
     ../../worksheet.cpp
     ../../worksheettextitem.cpp
     documentationpanelplugin.cpp
     documentationpanelwidget.cpp
)

add_panel(documentationpanelplugin ${DocumentationPanelPlugin_SRCS})

target_link_libraries(cantor_documentationpanelplugin
    cantorpart
    cantorlibs
    Qt5::Widgets
    Qt5::Help
    Qt5::Gui
    Qt5::Core
    Qt5::WebKitWidgets
    Qt5::WebEngine
    Qt5::WebEngineWidgets
    Qt5::Xml)
