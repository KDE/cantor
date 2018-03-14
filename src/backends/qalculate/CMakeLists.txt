add_subdirectory(plotassistant)
project(cantor-qalculate-backend)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS} -Wno-deprecated")
add_definitions(${QALCULATE_CFLAGS})

set( QalculateBackend_SRCS
qalculatesyntaxhelpobject.cpp
qalculatebackend.cpp
qalculatesession.cpp
qalculatehighlighter.cpp
qalculateexpression.cpp
qalculateextensions.cpp
qalculatecompletionobject.cpp
qalculateextensions.cpp
)

kconfig_add_kcfg_files(QalculateBackend_SRCS settings.kcfgc)
install(FILES qalculatebackend.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

ki18n_wrap_ui(QalculateBackend_SRCS settings.ui)

add_backend(qalculatebackend ${QalculateBackend_SRCS})

target_link_libraries( cantor_qalculatebackend cantorlibs ${QALCULATE_LIBRARIES} )

install( FILES cantor_qalculate.knsrc  DESTINATION  ${KDE_INSTALL_CONFDIR} )
