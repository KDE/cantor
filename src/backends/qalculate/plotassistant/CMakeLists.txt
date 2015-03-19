set( QalculatePlotAssistant_SRCS
     qalculateplotassistant.cpp
)

kconfig_add_kcfg_files(QalculatePlotAssistant_SRCS ../settings.kcfgc)
ki18n_wrap_ui(QalculatePlotAssistant_SRCS qalculateplotdialog.ui)

add_library( cantor_qalculateplotassistant MODULE ${QalculatePlotAssistant_SRCS} )
target_link_libraries( cantor_qalculateplotassistant cantorlibs)

install( FILES qalculateplotassistant.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/cantor)
install( FILES cantor_qalculateplotassistant.rc  DESTINATION  ${KDE_INSTALL_DATADIR}/cantor )
install(TARGETS cantor_qalculateplotassistant DESTINATION ${KDE_INSTALL_PLUGINDIR})
