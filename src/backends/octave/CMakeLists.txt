set( OctaveBackend_SRCS
  octavebackend.cpp
  octavesession.cpp
  octaveexpression.cpp
  octaveextensions.cpp
  octavehighlighter.cpp
  octavekeywords.cpp
  octavecompletionobject.cpp
  octavesyntaxhelpobject.cpp
  octavevariablemodel.cpp
)

add_subdirectory(scripts)

if (WITH_EPS)
    set(DEFAULT_PLOT_FORMAT "eps")
    set(EPS_PLOT_FORMAT_CHOICE "<choice name=\"eps\"/>")
    file(READ with_eps_ui_part.txt EPS_PLOT_FORMAT_UI_ELEMENT)
else (WITH_EPS)
    set(DEFAULT_PLOT_FORMAT "png")
endif (WITH_EPS)
configure_file(octavebackend.kcfg.in ${CMAKE_CURRENT_BINARY_DIR}/octavebackend.kcfg)
configure_file(settings.ui.in ${CMAKE_CURRENT_BINARY_DIR}/settings.ui)

kconfig_add_kcfg_files(OctaveBackend_SRCS settings.kcfgc)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/octavebackend.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

ki18n_wrap_ui(OctaveBackend_SRCS ${CMAKE_CURRENT_BINARY_DIR}/settings.ui)

add_backend(octavebackend ${OctaveBackend_SRCS})

target_link_libraries(cantor_octavebackend
  KF5::KIOCore
  KF5::ConfigCore
  KF5::ConfigGui
  KF5::SyntaxHighlighting
)


if(BUILD_TESTING)
  add_executable( testoctave testoctave.cpp ${OctaveBackend_SRCS})
  add_test(NAME testoctave COMMAND testoctave)
  ecm_mark_as_test(testoctave)
  target_link_libraries( testoctave
    Qt5::Test
    KF5::SyntaxHighlighting
    cantorlibs
    cantortest
  )
endif(BUILD_TESTING)

install(FILES cantor_octave.knsrc  DESTINATION  ${KDE_INSTALL_CONFDIR} )

install(FILES graphic_packages.xml DESTINATION ${KDE_INSTALL_DATADIR}/cantor/octave)
