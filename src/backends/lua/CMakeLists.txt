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
install(FILES luabackend.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

ki18n_wrap_ui(LuaBackend_SRCS settings.ui)

add_backend(luabackend ${LuaBackend_SRCS})

include_directories(${LUAJIT_INCLUDE_DIR})

target_link_libraries(cantor_luabackend ${LUAJIT_LIBRARY})

if(BUILD_TESTING)
  add_executable( testlua testlua.cpp)
  add_test(testlua testlua)
  ecm_mark_as_test(testlua)
  target_link_libraries( testlua
    Qt5::Test
    cantorlibs
    cantortest
  )
endif(BUILD_TESTING)

install(FILES cantor_lua.knsrc  DESTINATION  ${KDE_INSTALL_CONFDIR} )
