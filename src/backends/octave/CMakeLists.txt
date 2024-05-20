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

set(DEFAULT_PLOT_FORMAT "png")

configure_file(octavebackend.kcfg.in ${CMAKE_CURRENT_BINARY_DIR}/octavebackend.kcfg)

kconfig_add_kcfg_files(OctaveBackend_SRCS settings.kcfgc)
install(FILES octavebackend.kcfg.in DESTINATION ${KDE_INSTALL_KCFGDIR})

ki18n_wrap_ui(OctaveBackend_SRCS settings.ui)

add_backend(octavebackend ${OctaveBackend_SRCS})

target_link_libraries(cantor_octavebackend
  KF5::KIOCore
  KF5::ConfigCore
  KF5::ConfigGui
  KF5::SyntaxHighlighting
)

if (ENABLE_EMBEDDED_DOCUMENTATION)
    target_link_libraries(cantor_octavebackend cantor_help)
endif ()

if(BUILD_TESTING)
  add_executable(testoctave testoctave.cpp ${OctaveBackend_SRCS})
  add_test(NAME testoctave COMMAND testoctave)
  target_link_libraries(testoctave
    Qt5::Test
    KF5::SyntaxHighlighting
    KF5::NewStuffCore
    cantorlibs
    cantortest
  )
  if (ENABLE_EMBEDDED_DOCUMENTATION)
      target_link_libraries(testoctave cantor_help)
  endif ()
endif()


install(FILES cantor_octave.knsrc  DESTINATION  ${KDE_INSTALL_KNSRCDIR} )

install(FILES graphic_packages.xml DESTINATION ${KDE_INSTALL_DATADIR}/cantor/octave)
