set( ImportPackageAssistant_SRCS
     importpackageassistant.cpp
     importpackageassistant.qrc
)

ki18n_wrap_ui(ImportPackageAssistant_SRCS importpackagedlg.ui)

add_assistant(importpackageassistant ${ImportPackageAssistant_SRCS})
