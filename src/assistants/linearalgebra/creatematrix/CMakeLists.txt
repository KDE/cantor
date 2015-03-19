set( CreateMatrixAssistant_SRCS
     creatematrixassistant.cpp
     creatematrixdlg.cpp
)

ki18n_wrap_ui(CreateMatrixAssistant_SRCS creatematrixdlg.ui)

add_library( cantor_creatematrixassistant MODULE ${CreateMatrixAssistant_SRCS} )
target_link_libraries( cantor_creatematrixassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES creatematrixassistant.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/cantor)
install( FILES cantor_create_matrix_assistant.rc  DESTINATION  ${KDE_INSTALL_DATADIR}/cantor )
install(TARGETS cantor_creatematrixassistant DESTINATION ${KDE_INSTALL_PLUGINDIR})
