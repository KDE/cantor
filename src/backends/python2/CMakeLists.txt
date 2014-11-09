set( Python2Backend_SRCS
     ../python/pythonbackend.cpp
     ../python/pythonsession.cpp
     ../python/pythonexpression.cpp
     ../python/pythonkeywords.cpp
     ../python/pythonhighlighter.cpp
     ../python/pythoncompletionobject.cpp
     ../python/pythonextensions.cpp
)

if(MSVC)
    # ssize_t is typedef'd in both kdewin and python headers, this prevents using the kdewin one
    add_definitions(-D_SSIZE_T_DEFINED)
endif(MSVC)

include_directories(${PYTHON_LIBRARIES_DIR})
include_directories(${PYTHON_INCLUDE_DIR})

kconfig_add_kcfg_files(Python2Backend_SRCS ../python/settings.kcfgc)
install(FILES ../python/pythonbackend.kcfg DESTINATION ${KCFG_INSTALL_DIR})

ki18n_wrap_ui(Python2Backend_SRCS ../python/settings.ui)

add_library(cantor_python2backend MODULE ${Python2Backend_SRCS})
target_link_libraries(cantor_python2backend
    ${PYTHON_LIBRARIES}
    cantorlibs
    KF5::KIOCore
    KF5::ConfigCore
    KF5::ConfigGui)

kde4_add_unit_test(testpython2 testpython2.cpp)
target_link_libraries( testpython2 ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} cantorlibs cantortest )

install(FILES python2backend.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install(FILES ../python/keywords.xml DESTINATION ${DATA_INSTALL_DIR}/cantor/python2backend)
install(TARGETS cantor_python2backend DESTINATION ${PLUGIN_INSTALL_DIR})
