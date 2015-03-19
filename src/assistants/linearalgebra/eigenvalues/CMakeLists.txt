set( EigenValuesAssistant_SRCS
     eigenvaluesassistant.cpp
)

ki18n_wrap_ui(EigenValuesAssistant_SRCS eigenvaluesdlg.ui)

add_library( cantor_eigenvaluesassistant MODULE ${EigenValuesAssistant_SRCS} )
target_link_libraries( cantor_eigenvaluesassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES eigenvaluesassistant.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/cantor)
install( FILES cantor_eigenvalues_assistant.rc  DESTINATION  ${KDE_INSTALL_DATADIR}/cantor )
install(TARGETS cantor_eigenvaluesassistant DESTINATION ${KDE_INSTALL_PLUGINDIR})
