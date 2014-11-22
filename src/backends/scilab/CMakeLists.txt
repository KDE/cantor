set( ScilabBackend_SRCS
     scilabbackend.cpp
     scilabsession.cpp
     scilabexpression.cpp
     scilabextensions.cpp
     scilabkeywords.cpp
     scilabhighlighter.cpp
     scilabcompletionobject.cpp
)

kconfig_add_kcfg_files(ScilabBackend_SRCS settings.kcfgc)
install(FILES scilabbackend.kcfg DESTINATION ${KCFG_INSTALL_DIR})

ki18n_wrap_ui(ScilabBackend_SRCS settings.ui)

add_library(cantor_scilabbackend MODULE ${ScilabBackend_SRCS})
target_link_libraries(cantor_scilabbackend cantorlibs KF5::KIOCore)

install(FILES scilabbackend.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install(TARGETS cantor_scilabbackend DESTINATION ${PLUGIN_INSTALL_DIR})
