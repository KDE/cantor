set( ImportPackageAssistant_SRCS
     importpackageassistant.cpp
)

ki18n_wrap_ui(ImportPackageAssistant_SRCS importpackagedlg.ui)

add_library(cantor_importpackageassistant MODULE ${ImportPackageAssistant_SRCS})
target_link_libraries( cantor_importpackageassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES importpackageassistant.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install( FILES cantor_import_package_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/cantor )
install(TARGETS cantor_importpackageassistant DESTINATION ${PLUGIN_INSTALL_DIR})
