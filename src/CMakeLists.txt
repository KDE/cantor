#########################################################################
# Subdirectories
#########################################################################
add_subdirectory(lib)

include_directories( lib )
add_subdirectory(backends)
add_subdirectory(assistants)

set(mathematik_SRCS
   main.cpp
   mathematik.cpp
   helpextension.cpp
   backendchoosedialog.cpp
 )

kde4_add_kcfg_files(mathematik_SRCS settings.kcfgc)
install(FILES mathematik.kcfg DESTINATION ${KCFG_INSTALL_DIR})

kde4_add_ui_files(mathematik_SRCS settings.ui)
kde4_add_ui_files(mathematik_SRCS backendchooser.ui)

kde4_add_executable(mathematik ${mathematik_SRCS})

target_link_libraries(mathematik ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KNEWSTUFF2_LIBS} mathematiklibs)

########### install files ###############
install(TARGETS mathematik ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES mathematik.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES mathematik_shell.rc  DESTINATION  ${DATA_INSTALL_DIR}/mathematik )

install( FILES mathematik.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )


#########################################################################
# KPART SECTION
#########################################################################
#kde_module_LTLIBRARIES = libmathematikpart.la
set(mathematik_PART_SRCS
   mathematik_part.cpp
   worksheet.cpp
   worksheetentry.cpp
   helpextension.cpp
   resultproxy.cpp
 )

kde4_add_kcfg_files(mathematik_SRCS settings.kcfgc)
kde4_add_kcfg_files(mathematik_PART_SRCS settings.kcfgc)

kde4_add_plugin(mathematikpart WITH_PREFIX ${mathematik_PART_SRCS})

target_link_libraries(mathematikpart ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KNEWSTUFF2_LIBS} ${LIBSPECTRE_LIBRARY} mathematiklibs )

install( FILES mathematik_part.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES mathematik_part.rc  DESTINATION  ${DATA_INSTALL_DIR}/mathematikpart )

install(TARGETS mathematikpart  DESTINATION ${PLUGIN_INSTALL_DIR} )
