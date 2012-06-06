#########################################################################
# Subdirectories
#########################################################################
add_subdirectory(lib)

include_directories( lib ${CMAKE_CURRENT_BINARY_DIR}/lib)
if(KDE4_BUILD_TESTS)
  include_directories( lib/test )
endif(KDE4_BUILD_TESTS)

if(LIBSPECTRE_FOUND)
   include_directories(${LIBSPECTRE_INCLUDE_DIR})
endif(LIBSPECTRE_FOUND)
add_subdirectory(backends)
add_subdirectory(assistants)
add_subdirectory(xslt)
add_subdirectory(panelplugins)


#build the config object in a separate library, shared between shell and part
kde4_add_kcfg_files(config_SRCS settings.kcfgc)
kde4_add_library( cantor_config  SHARED ${config_SRCS} )
target_link_libraries( cantor_config ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KNEWSTUFF3_LIBS} )
install( TARGETS cantor_config  ${INSTALL_TARGETS_DEFAULT_ARGS} )

set(cantor_SRCS
   main.cpp
   cantor.cpp
   backendchoosedialog.cpp
 )

install(FILES cantor.kcfg DESTINATION ${KCFG_INSTALL_DIR})

kde4_add_ui_files(cantor_SRCS settings.ui)
kde4_add_ui_files(cantor_SRCS backendchooser.ui)
kde4_add_ui_files(cantor_SRCS imagesettings.ui)

kde4_add_app_icon(cantor_SRCS
   "${CMAKE_CURRENT_SOURCE_DIR}/../icons/hi*-app-cantor.png")
kde4_add_executable(cantor ${cantor_SRCS})

target_link_libraries(cantor ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KNEWSTUFF3_LIBS} cantorlibs cantor_config)

########### install files ###############
install(TARGETS cantor ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( PROGRAMS cantor.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES cantor_shell.rc  DESTINATION  ${DATA_INSTALL_DIR}/cantor )

install( FILES cantor.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )
install( FILES cantor_maxima.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )
install( FILES cantor_sage.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )
install( FILES cantor_kalgebra.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )
if (R_FOUND)
  install( FILES cantor_r.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )
endif (R_FOUND)

#########################################################################
# KPART SECTION
#########################################################################
#kde_module_LTLIBRARIES = libcantorpart.la
set(cantor_PART_SRCS
   cantor_part.cpp
   worksheet.cpp
   worksheetview.cpp
   worksheetentry.cpp
   worksheettextitem.cpp
   worksheetimageitem.cpp
   commandentry.cpp
   textentry.cpp
   #pagebreakentry.cpp
   #imageentry.cpp
   latexentry.cpp
   imagesettingsdialog.cpp
   scripteditorwidget.cpp
   resultitem.cpp
   textresultitem.cpp
   imageresultitem.cpp
   animationresultitem.cpp
   loadedexpression.cpp
   animationhandler.cpp
   animation.cpp
   epsrenderer.cpp
 )

configure_file (config-cantor.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-cantor.h )

kde4_add_plugin(cantorpart WITH_PREFIX ${cantor_PART_SRCS})

#add_library(blahtexcore STATIC IMPORTED)
#set_property(TARGET blahtexcore PROPERTY IMPORTED_LOCATION Path_To_BlahtexCore_Library/libblahtexcore.a)

target_link_libraries(cantorpart ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KNEWSTUFF3_LIBS}
				     ${KDE4_KTEXTEDITOR_LIBS} ${QT_QTXMLPATTERNS_LIBRARY}
				     cantorlibs cantor_config )
if(LIBSPECTRE_FOUND)
   target_link_libraries(cantorpart ${LIBSPECTRE_LIBRARY})
endif(LIBSPECTRE_FOUND)

install( FILES cantor_part.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor )
install( FILES cantor_part.rc  DESTINATION  ${DATA_INSTALL_DIR}/cantor )
install( FILES cantor_scripteditor.rc  DESTINATION ${DATA_INSTALL_DIR}/cantor )

install(TARGETS cantorpart  DESTINATION ${PLUGIN_INSTALL_DIR} )
