set( PythonBackend_SRCS
     pythonbackend.cpp
     pythonsession.cpp
     pythonexpression.cpp
     pythonkeywords.cpp
     pythonhighlighter.cpp
     pythoncompletionobject.cpp
     pythonextensions.cpp
     )

qt5_add_resources(PythonBackend_RSCS python.qrc)

ki18n_wrap_ui(PythonBackend_SRCS settings.ui)

add_library(cantor_pythonbackend SHARED ${PythonBackend_SRCS} ${PythonBackend_RSCS})
target_link_libraries(cantor_pythonbackend
    cantorlibs
    KF5::KIOCore
    KF5::ConfigCore
    KF5::ConfigGui)

install(TARGETS cantor_pythonbackend DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})
#install(FILES keywords.xml DESTINATION ${DATA_INSTALL_DIR}/cantor/pythonbackend)
#install(DIRECTORY . DESTINATION ${DATA_INSTALL_DIR}/cantor/pythonbackend FILES_MATCHING PATTERN "*.py")