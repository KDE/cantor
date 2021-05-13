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
  octavesettingswidget.cpp
  ../backendsettingswidget.cpp
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

kconfig_add_kcfg_files(OctaveBackend_SRCS settings.kcfgc)
install(FILES octavebackend.kcfg.in DESTINATION ${KDE_INSTALL_KCFGDIR})

ki18n_wrap_ui(OctaveBackend_SRCS settings.ui)

add_backend(octavebackend ${OctaveBackend_SRCS})

target_link_libraries(cantor_octavebackend cantor_help
  KF5::KIOCore
  KF5::ConfigCore
  KF5::ConfigGui
  KF5::SyntaxHighlighting
)

if(BUILD_TESTING)
  add_executable( testoctave testoctave.cpp ${OctaveBackend_SRCS})
  add_test(NAME testoctave COMMAND testoctave)
  target_link_libraries( testoctave
    Qt5::Test
    KF5::SyntaxHighlighting
    cantorlibs
    cantortest
    cantor_help
  )
endif()


install(FILES cantor_octave.knsrc  DESTINATION  ${KDE_INSTALL_KNSRCDIR} )


install(FILES graphic_packages.xml DESTINATION ${KDE_INSTALL_DATADIR}/cantor/octave)
