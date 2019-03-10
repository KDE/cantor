set( PythonBackend_SRCS
  pythonbackend.cpp
  pythonsession.cpp
  pythonexpression.cpp
  pythonkeywords.cpp
  pythonvariablemodel.cpp
  pythonhighlighter.cpp
  pythoncompletionobject.cpp
  pythonextensions.cpp
)

qt5_add_resources(PythonBackend_RSCS python.qrc)

ki18n_wrap_ui(PythonBackend_SRCS settings.ui)

add_library(cantor_pythonbackend SHARED ${PythonBackend_SRCS} ${PythonBackend_RSCS})
generate_export_header(cantor_pythonbackend)

target_link_libraries(cantor_pythonbackend
  cantorlibs
  KF5::KIOCore
  KF5::ConfigCore
  KF5::ConfigGui
  KF5::SyntaxHighlighting
  Qt5::DBus
)

install(TARGETS cantor_pythonbackend DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
#install(DIRECTORY . DESTINATION ${KDE_INSTALL_DATADIR}/cantor/pythonbackend FILES_MATCHING PATTERN "*.py")
