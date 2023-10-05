set( KAlgebraBackend_SRCS
  kalgebrasyntaxhelpobject.cpp
  kalgebrabackend.cpp
  kalgebrasession.cpp
  kalgebraexpression.cpp
  kalgebracompletionobject.cpp
  kalgebraextensions.cpp
)

remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

kconfig_add_kcfg_files(KAlgebraBackend_SRCS settings.kcfgc)
install(FILES kalgebrabackend.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

ki18n_wrap_ui(KAlgebraBackend_SRCS settings.ui)

add_backend(kalgebrabackend ${KAlgebraBackend_SRCS})

target_link_libraries( cantor_kalgebrabackend KF6::Analitza KF6::AnalitzaGui )

install( FILES cantor_kalgebra.knsrc  DESTINATION  ${KDE_INSTALL_KNSRCDIR} )
