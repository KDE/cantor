set( ImportPackageAssistant_SRCS
     importpackageassistant.cpp
)

kde4_add_ui_files(ImportPackageAssistant_SRCS importpackagedlg.ui)

kde4_add_plugin( cantor_importpackageassistant ${ImportPackageAssistant_SRCS} )
target_link_libraries( cantor_importpackageassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES importpackageassistant.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install( FILES cantor_import_package_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/cantor )
install(TARGETS cantor_importpackageassistant DESTINATION ${PLUGIN_INSTALL_DIR})
