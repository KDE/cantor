set( EigenVectorsAssistant_SRCS
     eigenvectorsassistant.cpp
     eigenvectorsassistant.qrc
)

ki18n_wrap_ui(EigenVectorsAssistant_SRCS eigenvectorsdlg.ui)

add_assistant(eigenvectorsassistant ${EigenVectorsAssistant_SRCS})
