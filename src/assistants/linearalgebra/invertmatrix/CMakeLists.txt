set( InvertMatrixAssistant_SRCS
     invertmatrixassistant.cpp
)

ki18n_wrap_ui(InvertMatrixAssistant_SRCS invertmatrixdlg.ui)

add_library( cantor_invertmatrixassistant MODULE ${InvertMatrixAssistant_SRCS} )
target_link_libraries( cantor_invertmatrixassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES invertmatrixassistant.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/cantor)
install( FILES cantor_invert_matrix_assistant.rc  DESTINATION  ${KDE_INSTALL_DATADIR}/cantor )
install(TARGETS cantor_invertmatrixassistant DESTINATION ${KDE_INSTALL_PLUGINDIR})
