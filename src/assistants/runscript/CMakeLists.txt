set( RunScriptAssistant_SRCS
     runscriptassistant.cpp
)

add_library( cantor_runscriptassistant MODULE ${RunScriptAssistant_SRCS} )
target_link_libraries( cantor_runscriptassistant ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} cantorlibs)

install( FILES runscriptassistant.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/cantor)
install( FILES cantor_runscript_assistant.rc  DESTINATION  ${KDE_INSTALL_DATADIR}/cantor )
install(TARGETS cantor_runscriptassistant DESTINATION ${KDE_INSTALL_PLUGINDIR})
