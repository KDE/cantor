add_subdirectory(plotassistant)
project(cantor-qalculate-backend)

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
  qalculatesettingswidget.cpp
  ../backendsettingswidget.cpp
)

kconfig_add_kcfg_files(QalculateBackend_SRCS settings.kcfgc)
install(FILES qalculatebackend.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

ki18n_wrap_ui(QalculateBackend_SRCS settings.ui)

add_backend(qalculatebackend ${QalculateBackend_SRCS})

target_link_libraries( cantor_qalculatebackend cantorlibs ${QALCULATE_LIBRARIES})

if (ENABLE_EMBEDDED_DOCUMENTATION)
    target_link_libraries(cantor_qalculatebackend cantor_help)
endif ()

install( FILES cantor_qalculate.knsrc  DESTINATION  ${KDE_INSTALL_KNSRCDIR} )
