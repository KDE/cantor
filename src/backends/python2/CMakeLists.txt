set( Python2Backend_SRCS
     python2backend.cpp
     python2session.cpp
     python2expression.cpp
     python2keywords.cpp
     python2highlighter.cpp
     python2completionobject.cpp
     python2extensions.cpp
)

if(MSVC)
    # ssize_t is typedef'd in both kdewin and python headers, this prevents using the kdewin one
    add_definitions(-D_SSIZE_T_DEFINED)
endif(MSVC)

include_directories(${PYTHON_LIBRARIES_DIR})
include_directories(${PYTHON_INCLUDE_DIR})

kconfig_add_kcfg_files(Python2Backend_SRCS settings.kcfgc)
install(FILES python2backend.kcfg DESTINATION ${KCFG_INSTALL_DIR})

ki18n_wrap_ui(Python2Backend_SRCS settings.ui)

add_library(cantor_python2backend MODULE ${Python2Backend_SRCS})
target_link_libraries(cantor_python2backend
    ${PYTHON_LIBRARIES}
    cantorlibs
    KF5::KIOCore
    KF5::ConfigCore
    KF5::ConfigGui)

install(FILES python2backend.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install(FILES keywords.xml DESTINATION ${DATA_INSTALL_DIR}/cantor/python2backend)
install(TARGETS cantor_python2backend DESTINATION ${PLUGIN_INSTALL_DIR})
