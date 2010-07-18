find_package(KDE4 REQUIRED KIO KDIRWATCH)

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
configure_file (octave-backend-config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/octave-backend-config.h )
add_subdirectory(scripts)

kde4_add_kcfg_files(OctaveBackend_SRCS settings.kcfgc)
install(FILES octavebackend.kcfg DESTINATION ${KCFG_INSTALL_DIR})

kde4_add_ui_files(OctaveBackend_SRCS settings.ui)

kde4_add_plugin( cantor_octavebackend ${OctaveBackend_SRCS} )
target_link_libraries( cantor_octavebackend ${KDE4_KDEUI_LIBS} cantorlibs ${KDE4_KIO_LIBS})

kde4_add_unit_test( testoctave testoctave.cpp)
target_link_libraries( testoctave
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
  cantorlibs
  cantortest
)

install( FILES octavebackend.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install(TARGETS cantor_octavebackend DESTINATION ${PLUGIN_INSTALL_DIR})
