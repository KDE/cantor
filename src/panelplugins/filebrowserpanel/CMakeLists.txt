set(FileBrowserPanelPlugin_SRCS
     filebrowserpanelplugin.cpp
)

add_panel(filebrowserpanelplugin ${FileBrowserPanelPlugin_SRCS})

target_link_libraries(cantor_filebrowserpanelplugin KF5::Parts)
