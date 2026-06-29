set(FileBrowserPanelPlugin_SRCS
     tocpanelplugin.cpp
)

add_panel(tocpanelplugin ${FileBrowserPanelPlugin_SRCS})

target_link_libraries(cantor_tocpanelplugin cantor_config)
target_include_directories(cantor_tocpanelplugin PRIVATE ${CMAKE_BINARY_DIR}/src)
