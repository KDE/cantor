set( RunScriptAssistant_SRCS 
     runscriptassistant.cpp
)

kde4_add_plugin( cantor_runscriptassistant ${RunScriptAssistant_SRCS} )
target_link_libraries( cantor_runscriptassistant ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} cantorlibs)

install( FILES runscriptassistant.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install( FILES cantor_runscript_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/cantor )
install(TARGETS cantor_runscriptassistant DESTINATION ${PLUGIN_INSTALL_DIR})
