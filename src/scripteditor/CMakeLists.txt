set( scripteditor_SRCS
    main.cpp
    scripteditorwidget.cpp

    scripteditor.qrc
)

add_executable( cantor_scripteditor ${scripteditor_SRCS} )

target_link_libraries(cantor_scripteditor KF6::TextEditor KF6::I18n)

install(TARGETS cantor_scripteditor ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
