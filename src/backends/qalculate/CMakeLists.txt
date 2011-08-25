project(cantor-qalculate-backend)

set(CMAKE_CXX_FLAGS "${KDE4_ENABLE_EXCEPTIONS} -Wno-deprecated")
add_definitions(${QALCULATE_CFLAGS})

set( QalculateBackend_SRCS
qalculatesyntaxhelpobject.cpp
qalculatebackend.cpp
qalculatesession.cpp
qalculatehighlighter.cpp
qalculateexpression.cpp
qalculatecompletionobject.cpp
qalculateextensions.cpp
settingswidget.cpp
)

kde4_add_kcfg_files(QalculateBackend_SRCS settings.kcfgc)
install(FILES qalculatebackend.kcfg DESTINATION ${KCFG_INSTALL_DIR})

kde4_add_ui_files(QalculateBackend_SRCS settings.ui)

kde4_add_plugin( cantor_qalculatebackend ${QalculateBackend_SRCS} )
message(STATUS ${CLN_LIBRARIES})
target_link_libraries( cantor_qalculatebackend ${KDE4_KDEUI_LIBS} cantorlibs ${QALCULATE_LIBRARIES} ${CLN_LIBRARIES})

install( FILES qalculatebackend.desktop DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install(TARGETS cantor_qalculatebackend DESTINATION ${PLUGIN_INSTALL_DIR})
