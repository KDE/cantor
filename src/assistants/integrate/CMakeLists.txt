set( IntegrateAssistant_SRCS
     integrateassistant.cpp
)

ki18n_wrap_ui(IntegrateAssistant_SRCS integratedlg.ui)

add_library( cantor_integrateassistant MODULE ${IntegrateAssistant_SRCS} )
target_link_libraries( cantor_integrateassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES integrateassistant.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/cantor)
install( FILES cantor_integrate_assistant.rc  DESTINATION  ${KDE_INSTALL_DATADIR}/cantor )
install(TARGETS cantor_integrateassistant DESTINATION ${KDE_INSTALL_PLUGINDIR})
