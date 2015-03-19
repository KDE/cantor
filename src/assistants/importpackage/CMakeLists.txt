set( ImportPackageAssistant_SRCS
     importpackageassistant.cpp
)

ki18n_wrap_ui(ImportPackageAssistant_SRCS importpackagedlg.ui)

add_library(cantor_importpackageassistant MODULE ${ImportPackageAssistant_SRCS})
target_link_libraries( cantor_importpackageassistant ${KDE4_KDEUI_LIBS} cantorlibs)

install( FILES importpackageassistant.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/cantor)
install( FILES cantor_import_package_assistant.rc  DESTINATION  ${KDE_INSTALL_DATADIR}/cantor )
install(TARGETS cantor_importpackageassistant DESTINATION ${KDE_INSTALL_PLUGINDIR})
