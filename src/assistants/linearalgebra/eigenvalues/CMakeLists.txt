set( EigenValuesAssistant_SRCS
     eigenvaluesassistant.cpp
)

ki18n_wrap_ui(EigenValuesAssistant_SRCS eigenvaluesdlg.ui)

add_assistant(eigenvaluesassistant ${EigenValuesAssistant_SRCS})

install( FILES cantor_eigenvalues_assistant.rc  DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/cantor )
