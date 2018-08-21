set( scripteditor_SRCS
    main.cpp
    scripteditorwidget.cpp
)

add_executable( cantor_scripteditor ${scripteditor_SRCS} )

target_link_libraries(cantor_scripteditor KF5::TextEditor)

install(TARGETS cantor_scripteditor ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
