add_definitions(-DBUILD_WITH_PYTHON3)

set( Python3Backend_SRCS
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

string(REPLACE "-l" "" PYTHON3_LIB_NAME ${PYTHONLIBS3_LDFLAGS})
add_definitions(-DPYTHON3_LIB_NAME="${PYTHON3_LIB_NAME}")

include_directories(${PYTHONLIBS3_INCLUDE_DIRS})

kconfig_add_kcfg_files(Python3Backend_SRCS ../python/settings.kcfgc)
install(FILES ../python/pythonbackend.kcfg DESTINATION ${KCFG_INSTALL_DIR})

ki18n_wrap_ui(Python3Backend_SRCS ../python/settings.ui)

add_library(cantor_python3backend MODULE ${Python3Backend_SRCS})
target_link_libraries(cantor_python3backend
    ${PYTHONLIBS3_LIBRARIES}
    cantorlibs
    KF5::KIOCore
    KF5::ConfigCore
    KF5::ConfigGui)

install(FILES cantor_python3.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR})
install(FILES python3backend.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install(FILES ../python/keywords.xml DESTINATION ${DATA_INSTALL_DIR}/cantor/python3backend)
install(TARGETS cantor_python3backend DESTINATION ${PLUGIN_INSTALL_DIR})