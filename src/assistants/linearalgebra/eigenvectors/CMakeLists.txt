set( EigenVectorsAssistant_SRCS
     eigenvectorsassistant.cpp
)

ki18n_wrap_ui(EigenVectorsAssistant_SRCS eigenvectorsdlg.ui)

add_library( cantor_eigenvectorsassistant MODULE ${EigenVectorsAssistant_SRCS} )
target_link_libraries( cantor_eigenvectorsassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES eigenvectorsassistant.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/cantor)
install( FILES cantor_eigenvectors_assistant.rc  DESTINATION  ${KDE_INSTALL_DATADIR}/cantor )
install(TARGETS cantor_eigenvectorsassistant DESTINATION ${KDE_INSTALL_PLUGINDIR})
