find_package(KDE4 REQUIRED KIO KDIRWATCH)

set( Python2Backend_SRCS
     python2backend.cpp
     python2session.cpp
     python2expression.cpp
     python2keywords.cpp
     python2highlighter.cpp
     python2completionobject.cpp
     python2extensions.cpp
)

include_directories(${PYTHON_LIBRARIES_DIR})
include_directories(${PYTHON_INCLUDE_DIR})

kde4_add_kcfg_files(Python2Backend_SRCS settings.kcfgc)
install(FILES python2backend.kcfg DESTINATION ${KCFG_INSTALL_DIR})

kde4_add_ui_files(Python2Backend_SRCS settings.ui)

kde4_add_plugin(cantor_python2backend ${Python2Backend_SRCS})
target_link_libraries(cantor_python2backend ${PYTHON_LIBRARIES} ${KDE4_KDEUI_LIBS} cantorlibs ${KDE4_KIO_LIBS})

install(FILES python2backend.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install(FILES keywords.xml DESTINATION ${DATA_INSTALL_DIR}/cantor/python2backend)
install(TARGETS cantor_python2backend DESTINATION ${PLUGIN_INSTALL_DIR})
