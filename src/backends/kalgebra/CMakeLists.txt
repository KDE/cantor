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
install(FILES kalgebrabackend.kcfg DESTINATION ${KCFG_INSTALL_DIR})

ki18n_wrap_ui(KAlgebraBackend_SRCS settings.ui)

add_library( cantor_kalgebrabackend MODULE ${KAlgebraBackend_SRCS} )
target_link_libraries( cantor_kalgebrabackend cantorlibs KF5::Analitza KF5::AnalitzaGui)

install( FILES cantor_kalgebra.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )
install( FILES kalgebrabackend.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install(TARGETS cantor_kalgebrabackend DESTINATION ${PLUGIN_INSTALL_DIR})
