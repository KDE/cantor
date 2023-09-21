set( EigenValuesAssistant_SRCS
     eigenvaluesassistant.cpp
     eigenvaluesassistant.qrc
)

ki18n_wrap_ui(EigenValuesAssistant_SRCS eigenvaluesdlg.ui)

add_assistant(eigenvaluesassistant ${EigenValuesAssistant_SRCS})
