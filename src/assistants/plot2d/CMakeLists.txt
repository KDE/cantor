set( Plot2dAssistant_SRCS
     plot2dassistant.cpp
)

ki18n_wrap_ui(Plot2dAssistant_SRCS plot2ddlg.ui)

add_library( cantor_plot2dassistant MODULE ${Plot2dAssistant_SRCS} )
target_link_libraries( cantor_plot2dassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES plot2dassistant.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install( FILES cantor_plot2d_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/cantor )
install(TARGETS cantor_plot2dassistant DESTINATION ${PLUGIN_INSTALL_DIR})
