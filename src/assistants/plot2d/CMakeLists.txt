set( Plot2dAssistant_SRCS
     plot2dassistant.cpp
)

ki18n_wrap_ui(Plot2dAssistant_SRCS plot2ddlg.ui)

add_library( cantor_plot2dassistant MODULE ${Plot2dAssistant_SRCS} )
target_link_libraries( cantor_plot2dassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES plot2dassistant.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/cantor)
install( FILES cantor_plot2d_assistant.rc  DESTINATION  ${KDE_INSTALL_DATADIR}/cantor )
install(TARGETS cantor_plot2dassistant DESTINATION ${KDE_INSTALL_PLUGINDIR})
