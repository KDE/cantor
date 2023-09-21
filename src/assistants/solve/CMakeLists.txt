set( SolveAssistant_SRCS
     solveassistant.cpp
     solveassistant.qrc
)

ki18n_wrap_ui(SolveAssistant_SRCS solvedlg.ui)

add_assistant(solveassistant ${SolveAssistant_SRCS})
