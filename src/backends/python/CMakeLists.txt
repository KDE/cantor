find_package(KDE4 REQUIRED KIO KDIRWATCH)

set( PythonBackend_SRCS
     pythonbackend.cpp
     pythonsession.cpp
     pythonexpression.cpp
)

find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_LIBRARIES_DIR})
include_directories(${PYTHON_INCLUDE_DIR})

kde4_add_kcfg_files(PythonBackend_SRCS settings.kcfgc)
install(FILES pythonbackend.kcfg DESTINATION ${KCFG_INSTALL_DIR})

kde4_add_ui_files(PythonBackend_SRCS settings.ui)

kde4_add_plugin(cantor_pythonbackend ${PythonBackend_SRCS})
target_link_libraries(cantor_pythonbackend ${PYTHON_LIBRARIES} ${KDE4_KDEUI_LIBS} cantorlibs ${KDE4_KIO_LIBS})

install(FILES pythonbackend.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
# install(FILES keywords.xml DESTINATION ${DATA_INSTALL_DIR}/cantor/pythonbackend)
install(TARGETS cantor_pythonbackend DESTINATION ${PLUGIN_INSTALL_DIR})
