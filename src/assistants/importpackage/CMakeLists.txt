set( ImportPackageAssistant_SRCS
     importpackageassistant.cpp
)

ki18n_wrap_ui(ImportPackageAssistant_SRCS importpackagedlg.ui)

add_assistant(importpackageassistant ${ImportPackageAssistant_SRCS})

install(FILES cantor_import_package_assistant.rc  DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/cantor)
