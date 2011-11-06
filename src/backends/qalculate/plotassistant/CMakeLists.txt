set( QalculatePlotAssistant_SRCS 
     qalculateplotassistant.cpp
)

kde4_add_kcfg_files(QalculatePlotAssistant_SRCS ../settings.kcfgc)
kde4_add_ui_files(QalculatePlotAssistant_SRCS qalculateplotdialog.ui)

kde4_add_plugin( cantor_qalculateplotassistant ${QalculatePlotAssistant_SRCS} )
target_link_libraries( cantor_qalculateplotassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES qalculateplotassistant.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install( FILES cantor_qalculateplotassistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/cantor )
install(TARGETS cantor_qalculateplotassistant DESTINATION ${PLUGIN_INSTALL_DIR})
