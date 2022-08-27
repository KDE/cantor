set( RunScriptAssistant_SRCS
     runscriptassistant.cpp
)

add_assistant(runscriptassistant ${RunScriptAssistant_SRCS})

target_link_libraries( cantor_runscriptassistant cantorlibs)

install( FILES cantor_runscript_assistant.rc  DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/cantor )
