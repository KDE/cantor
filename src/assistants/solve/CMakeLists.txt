set( SolveAssistant_SRCS
     solveassistant.cpp
)

ki18n_wrap_ui(SolveAssistant_SRCS solvedlg.ui)

add_assistant(solveassistant ${SolveAssistant_SRCS})

install( FILES cantor_solve_assistant.rc  DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/cantor )
