set( EigenValuesAssistant_SRCS
     eigenvaluesassistant.cpp
)

ki18n_wrap_ui(EigenValuesAssistant_SRCS eigenvaluesdlg.ui)

add_library( cantor_eigenvaluesassistant MODULE ${EigenValuesAssistant_SRCS} )
target_link_libraries( cantor_eigenvaluesassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES eigenvaluesassistant.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install( FILES cantor_eigenvalues_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/cantor )
install(TARGETS cantor_eigenvaluesassistant DESTINATION ${PLUGIN_INSTALL_DIR})
