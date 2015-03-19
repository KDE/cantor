set( DifferentiateAssistant_SRCS
     differentiateassistant.cpp
)

ki18n_wrap_ui(DifferentiateAssistant_SRCS differentiatedlg.ui)

add_library( cantor_differentiateassistant MODULE ${DifferentiateAssistant_SRCS} )
target_link_libraries( cantor_differentiateassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES differentiateassistant.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/cantor)
install( FILES cantor_differentiate_assistant.rc  DESTINATION  ${KDE_INSTALL_DATADIR}/cantor )
install(TARGETS cantor_differentiateassistant DESTINATION ${KDE_INSTALL_PLUGINDIR})
