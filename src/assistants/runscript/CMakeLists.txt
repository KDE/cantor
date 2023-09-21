set( RunScriptAssistant_SRCS
     runscriptassistant.cpp
     runscriptassistant.qrc
)

add_assistant(runscriptassistant ${RunScriptAssistant_SRCS})

target_link_libraries( cantor_runscriptassistant cantorlibs)
