# find_package(KDE4 REQUIRED KIO KDIRWATCH)

set( OctaveBackend_SRCS
octavebackend.cpp
octavesession.cpp
octaveexpression.cpp
octaveextensions.cpp
octavehighlighter.cpp
octavecompletionobject.cpp
octavesyntaxhelpobject.cpp
)

set (OCTAVE_SCRIPT_INSTALL_DIR ${DATA_INSTALL_DIR}/cantor/octavebackend)
set (OCTAVE_SCRIPT_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/${OCTAVE_SCRIPT_INSTALL_DIR}/)
configure_file (octave-backend-config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/octave-backend-config.h )
add_subdirectory(scripts)

kconfig_add_kcfg_files(OctaveBackend_SRCS settings.kcfgc)
install(FILES octavebackend.kcfg DESTINATION ${KCFG_INSTALL_DIR})

ki18n_wrap_ui(OctaveBackend_SRCS settings.ui)

add_library(cantor_octavebackend MODULE ${OctaveBackend_SRCS} )
target_link_libraries(cantor_octavebackend
    cantorlibs
    KF5::KIOCore
    KF5::ConfigCore
    KF5::ConfigGui
    )


if(BUILD_TESTING)
 add_executable( testoctave testoctave.cpp)
 add_test(testoctave testoctave)
 ecm_mark_as_test(testoctave)
 target_link_libraries( testoctave
   KF5::KDELibs4Support
   Qt5::Test
   cantorlibs
   cantortest
 )
endif(BUILD_TESTING)

install(FILES octavebackend.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install(TARGETS cantor_octavebackend DESTINATION ${PLUGIN_INSTALL_DIR})
