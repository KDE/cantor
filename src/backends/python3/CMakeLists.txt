set( Python3Backend_SRCS
     ../python/pythonbackend.cpp
     ../python/pythonsession.cpp
     ../python/pythonexpression.cpp
     ../python/pythonkeywords.cpp
     ../python/pythonhighlighter.cpp
     ../python/pythoncompletionobject.cpp
     ../python/pythonextensions.cpp
     python3backend.cpp
     python3session.cpp
)

kconfig_add_kcfg_files(Python3Backend_SRCS ../python/settings.kcfgc)
install(FILES ../python/pythonbackend.kcfg DESTINATION ${KCFG_INSTALL_DIR})

ki18n_wrap_ui(Python3Backend_SRCS ../python/settings.ui)

add_library(cantor_python3backend MODULE ${Python3Backend_SRCS})
target_link_libraries(cantor_python3backend
    cantorlibs
    KF5::KIOCore
    KF5::ConfigCore
    KF5::ConfigGui
    Qt5::DBus)

install(FILES cantor_python3.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR})
install(FILES python3backend.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install(FILES ../python/keywords.xml DESTINATION ${DATA_INSTALL_DIR}/cantor/python3backend)
install(TARGETS cantor_python3backend DESTINATION ${PLUGIN_INSTALL_DIR})

add_subdirectory(python3server)