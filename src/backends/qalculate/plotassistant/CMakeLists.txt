set( QalculatePlotAssistant_SRCS
     qalculateplotassistant.cpp
)

kconfig_add_kcfg_files(QalculatePlotAssistant_SRCS ../settings.kcfgc)
ki18n_wrap_ui(QalculatePlotAssistant_SRCS qalculateplotdialog.ui)

add_library( cantor_qalculateplotassistant MODULE ${QalculatePlotAssistant_SRCS} )
target_link_libraries( cantor_qalculateplotassistant cantorlibs)

install( FILES qalculateplotassistant.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install( FILES cantor_qalculateplotassistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/cantor )
install(TARGETS cantor_qalculateplotassistant DESTINATION ${PLUGIN_INSTALL_DIR})
