set( RunScriptAssistant_SRCS 
     runscriptassistant.cpp
)

kde4_add_plugin( mathematik_runscriptassistant ${RunScriptAssistant_SRCS} )
target_link_libraries( mathematik_runscriptassistant ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} mathematiklibs)

install( FILES runscriptassistant.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/mathematik)
install( FILES mathematik_runscript_assistant.rc  DESTINATION  ${DATA_INSTALL_DIR}/mathematik )
install(TARGETS mathematik_runscriptassistant DESTINATION ${PLUGIN_INSTALL_DIR})
