set( QalculatePlotAssistant_SRCS
     qalculateplotassistant.cpp
)

kconfig_add_kcfg_files(QalculatePlotAssistant_SRCS ../settings.kcfgc)
ki18n_wrap_ui(QalculatePlotAssistant_SRCS qalculateplotdialog.ui)

kcoreaddons_add_plugin(cantor_qalculateplotassistant
    SOURCES ${QalculatePlotAssistant_SRCS}
    INSTALL_NAMESPACE "cantor/assistants")

target_link_libraries(cantor_qalculateplotassistant cantorlibs)

install( FILES cantor_qalculateplotassistant.rc  DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/cantor )
