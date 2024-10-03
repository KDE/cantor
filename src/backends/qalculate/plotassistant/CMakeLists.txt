set( QalculatePlotAssistant_SRCS
     qalculateplotassistant.cpp
     qalculateplotassistant.qrc
)

kconfig_add_kcfg_files(QalculatePlotAssistant_SRCS ../settings.kcfgc)
ki18n_wrap_ui(QalculatePlotAssistant_SRCS qalculateplotdialog.ui)

kcoreaddons_add_plugin(cantor_qalculateplotassistant
    SOURCES ${QalculatePlotAssistant_SRCS}
    INSTALL_NAMESPACE "cantor_plugins/assistants")

target_link_libraries(cantor_qalculateplotassistant cantorlibs)
