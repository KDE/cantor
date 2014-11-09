set(LuaBackend_SRCS
    luabackend.cpp
    luasession.cpp
    luaexpression.cpp
    luacompletionobject.cpp
    luahelper.cpp
    luahighlighter.cpp
    luaextensions.cpp
)

kconfig_add_kcfg_files(LuaBackend_SRCS settings.kcfgc)
install(FILES luabackend.kcfg DESTINATION ${KCFG_INSTALL_DIR})

ki18n_wrap_ui(LuaBackend_SRCS settings.ui)

add_library(cantor_luabackend MODULE ${LuaBackend_SRCS})

include_directories(${LUAJIT_INCLUDE_DIR})

target_link_libraries(cantor_luabackend ${LUAJIT_LIBRARY} cantorlibs)

install(FILES  luabackend.desktop DESTINATION ${SERVICES_INSTALL_DIR}/cantor)
install(TARGETS cantor_luabackend DESTINATION ${PLUGIN_INSTALL_DIR})
