set( Plot3dAssistant_SRCS
     plot3dassistant.cpp
)

ki18n_wrap_ui(Plot3dAssistant_SRCS plot3ddlg.ui)

add_library( cantor_plot3dassistant MODULE ${Plot3dAssistant_SRCS} )
target_link_libraries( cantor_plot3dassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES plot3dassistant.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install( FILES cantor_plot3d_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/cantor )
install(TARGETS cantor_plot3dassistant DESTINATION ${PLUGIN_INSTALL_DIR})
