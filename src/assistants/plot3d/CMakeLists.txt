set( Plot3dAssistant_SRCS
     plot3dassistant.cpp
)

ki18n_wrap_ui(Plot3dAssistant_SRCS plot3ddlg.ui)

add_library( cantor_plot3dassistant MODULE ${Plot3dAssistant_SRCS} )
target_link_libraries( cantor_plot3dassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES plot3dassistant.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/cantor)
install( FILES cantor_plot3d_assistant.rc  DESTINATION  ${KDE_INSTALL_DATADIR}/cantor )
install(TARGETS cantor_plot3dassistant DESTINATION ${KDE_INSTALL_PLUGINDIR})
