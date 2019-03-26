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

kconfig_add_kcfg_files(OctaveBackend_SRCS settings.kcfgc)
install(FILES octavebackend.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

ki18n_wrap_ui(OctaveBackend_SRCS settings.ui)

add_backend(octavebackend ${OctaveBackend_SRCS})

target_link_libraries(cantor_octavebackend
  KF5::KIOCore
  KF5::ConfigCore
  KF5::ConfigGui
  KF5::SyntaxHighlighting
)


if(BUILD_TESTING)
  add_executable( testoctave testoctave.cpp)
  add_test(testoctave testoctave)
  ecm_mark_as_test(testoctave)
  target_link_libraries( testoctave
    Qt5::Test
    cantorlibs
    cantortest
  )
endif(BUILD_TESTING)

install(FILES cantor_octave.knsrc  DESTINATION  ${KDE_INSTALL_CONFDIR} )
