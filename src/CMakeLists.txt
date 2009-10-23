#########################################################################
# Subdirectories
#########################################################################
add_subdirectory(lib)

include_directories( lib ${CMAKE_CURRENT_BINARY_DIR}/lib)
if(LIBSPECTRE_FOUND)
   include_directories(${LIBSPECTRE_INCLUDE_DIR})
endif(LIBSPECTRE_FOUND)
add_subdirectory(backends)
add_subdirectory(assistants)

set(cantor_SRCS
   main.cpp
   cantor.cpp
   backendchoosedialog.cpp
 )

kde4_add_kcfg_files(cantor_SRCS settings.kcfgc)
install(FILES cantor.kcfg DESTINATION ${KCFG_INSTALL_DIR})

kde4_add_ui_files(cantor_SRCS settings.ui)
kde4_add_ui_files(cantor_SRCS backendchooser.ui)

kde4_add_executable(cantor ${cantor_SRCS})

target_link_libraries(cantor ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KNEWSTUFF2_LIBS} cantorlibs)

########### install files ###############
install(TARGETS cantor ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES cantor.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES cantor_shell.rc  DESTINATION  ${DATA_INSTALL_DIR}/cantor )

install( FILES cantor.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )


#########################################################################
# KPART SECTION
#########################################################################
#kde_module_LTLIBRARIES = libcantorpart.la
set(cantor_PART_SRCS
   cantor_part.cpp
   worksheet.cpp
   worksheetentry.cpp
   scripteditorwidget.cpp
   resultproxy.cpp
   loadedexpression.cpp
   animationhandler.cpp
   animation.cpp
   resultcontextmenu.cpp
 )

kde4_add_kcfg_files(cantor_PART_SRCS settings.kcfgc)

configure_file (config-cantor.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-cantor.h )


kde4_add_plugin(cantorpart WITH_PREFIX ${cantor_PART_SRCS})

target_link_libraries(cantorpart ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KNEWSTUFF2_LIBS}
				     ${KDE4_KTEXTEDITOR_LIBS}
				     cantorlibs )
if(LIBSPECTRE_FOUND)
   target_link_libraries(cantorpart ${LIBSPECTRE_LIBRARY})
endif(LIBSPECTRE_FOUND)

install( FILES cantor_part.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES cantor_part.rc  DESTINATION  ${DATA_INSTALL_DIR}/cantorpart )

install(TARGETS cantorpart  DESTINATION ${PLUGIN_INSTALL_DIR} )
