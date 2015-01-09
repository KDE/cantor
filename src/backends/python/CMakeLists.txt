set( PythonBackend_SRCS
     pythonbackend.cpp
     pythonsession.cpp
     pythonexpression.cpp
     pythonkeywords.cpp
     pythonhighlighter.cpp
     pythoncompletionobject.cpp
     pythonextensions.cpp
     )

kconfig_add_kcfg_files(PythonBackend_SRCS settings.kcfgc)

ki18n_wrap_ui(PythonBackend_SRCS settings.ui)

add_library(cantor_pythonbackend SHARED ${PythonBackend_SRCS})
target_link_libraries(cantor_pythonbackend
    cantorlibs
    KF5::KIOCore
    KF5::ConfigCore
    KF5::ConfigGui)

install(TARGETS cantor_pythonbackend DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES pythonbackend.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES keywords.xml DESTINATION ${DATA_INSTALL_DIR}/cantor/pythonbackend)