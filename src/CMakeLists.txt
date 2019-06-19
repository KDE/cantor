#########################################################################
# Subdirectories
#########################################################################
add_subdirectory(lib)
add_subdirectory(scripteditor)

include_directories( lib ${CMAKE_CURRENT_BINARY_DIR}/lib)
if(BUILD_TESTING)
  include(ECMMarkAsTest)
  include_directories( lib/test )
endif(BUILD_TESTING)

if(LIBSPECTRE_FOUND)
   include_directories(${LIBSPECTRE_INCLUDE_DIR})
endif(LIBSPECTRE_FOUND)
add_subdirectory(backends)
add_subdirectory(assistants)
add_subdirectory(xslt)
add_subdirectory(panelplugins)


#build the config object in a separate library, shared between shell and part
kconfig_add_kcfg_files(config_SRCS settings.kcfgc)
add_library( cantor_config  SHARED ${config_SRCS} )
target_link_libraries( cantor_config  KF5::Parts KF5::NewStuff )
install( TARGETS cantor_config  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

set(cantor_SRCS
   main.cpp
   cantor.cpp
   backendchoosedialog.cpp
 )

install(FILES cantor.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

ki18n_wrap_ui(cantor_SRCS settings.ui)
ki18n_wrap_ui(cantor_SRCS backendchooser.ui)

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/*-apps-cantor.png")
ecm_add_app_icon(cantor_SRCS ICONS ${ICONS_SRCS})
add_executable(cantor ${cantor_SRCS})

target_link_libraries(cantor KF5::Parts KF5::NewStuff KF5::ConfigCore KF5::CoreAddons KF5::ConfigGui
    KF5::Crash KF5::XmlGui cantorlibs cantor_config)

########### install files ###############
install(TARGETS cantor ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
install( PROGRAMS org.kde.cantor.desktop  DESTINATION  ${KDE_INSTALL_APPDIR} )
install( FILES cantor_shell.rc  DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/cantor )
install( FILES cantor.knsrc  DESTINATION  ${KDE_INSTALL_CONFDIR} )

#########################################################################
# KPART SECTION
#########################################################################

set(cantor_PART_SRCS
   cantor_part.cpp
   worksheet.cpp
   worksheetview.cpp
   worksheetentry.cpp
   worksheettextitem.cpp
   worksheetimageitem.cpp
   commandentry.cpp
   textentry.cpp
   markdownentry.cpp
   pagebreakentry.cpp
   imageentry.cpp
   latexentry.cpp
   placeholderentry.cpp
   worksheetcursor.cpp
   searchbar.cpp
   actionbar.cpp
   worksheettoolbutton.cpp
   imagesettingsdialog.cpp
   scripteditor/scripteditorwidget.cpp
   resultitem.cpp
   textresultitem.cpp
   imageresultitem.cpp
   animationresultitem.cpp
   loadedexpression.cpp
   animation.cpp
   epsrenderer.cpp
   jupyterutils.cpp
 )

ki18n_wrap_ui(cantor_PART_SRCS imagesettings.ui)
ki18n_wrap_ui(cantor_PART_SRCS standardsearchbar.ui)
ki18n_wrap_ui(cantor_PART_SRCS extendedsearchbar.ui)

string(CONCAT PATH_TO_CANTOR_BACKENDS ${CMAKE_INSTALL_PREFIX} "/${PLUGIN_INSTALL_DIR}")

configure_file (config-cantor.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-cantor.h )

kcoreaddons_add_plugin(cantorpart
    SOURCES ${cantor_PART_SRCS}
    JSON "cantor_part.json"
    INSTALL_NAMESPACE ".")

set_target_properties(cantorpart PROPERTIES PREFIX "${CMAKE_SHARED_LIBRARY_PREFIX}")

target_link_libraries(cantorpart KF5::Parts KF5::NewStuff
                     KF5::TextEditor ${Qt5XmlPatterns_LIBRARIES}
                     KF5::KIOCore KF5::KIOFileWidgets KF5::KIOWidgets
                     Qt5::PrintSupport cantorlibs cantor_config )
if(LIBSPECTRE_FOUND)
    target_link_libraries(cantorpart ${LIBSPECTRE_LIBRARY})
endif(LIBSPECTRE_FOUND)
if(Discount_FOUND)
    target_link_libraries(cantorpart Discount::Lib)
endif(Discount_FOUND)

install( FILES cantor_part.rc  DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/cantor )
install( FILES cantor_scripteditor.rc  DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/cantor )
