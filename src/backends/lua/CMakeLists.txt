find_package(KDE4 REQUIRED KIO)

set(LuaBackend_SRCS
    luabackend.cpp
    luasession.cpp
    luaexpression.cpp
    luacompletionobject.cpp
    luahelper.cpp
    luahighlighter.cpp
    luaextensions.cpp
)

kde4_add_kcfg_files(LuaBackend_SRCS settings.kcfgc)
install(FILES luabackend.kcfg DESTINATION ${KCFG_INSTALL_DIR})

kde4_add_ui_files(LuaBackend_SRCS settings.ui)

kde4_add_plugin( cantor_luabackend ${LuaBackend_SRCS} )

include_directories(${LUAJIT_INCLUDE_DIR})

target_link_libraries( cantor_luabackend ${KDE4_KDEUI_LIBS} ${LUAJIT_LIBRARY} cantorlibs ${KDE4_KIO_LIBS})

install(FILES  luabackend.desktop DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install(TARGETS cantor_luabackend DESTINATION ${PLUGIN_INSTALL_DIR})
