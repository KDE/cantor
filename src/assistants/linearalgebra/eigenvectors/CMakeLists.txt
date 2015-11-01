set( EigenVectorsAssistant_SRCS
     eigenvectorsassistant.cpp
)

ki18n_wrap_ui(EigenVectorsAssistant_SRCS eigenvectorsdlg.ui)

add_assistant(eigenvectorsassistant ${EigenVectorsAssistant_SRCS})

install( FILES cantor_eigenvectors_assistant.rc  DESTINATION  ${KDE_INSTALL_DATADIR}/cantor )
