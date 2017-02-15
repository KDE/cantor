find_package(PkgConfig)

pkg_check_modules(LUAJIT QUIET luajit)

find_path(LUAJIT_INCLUDE_DIR lua.hpp HINTS ${LUAJIT_INCLUDEDIR} ${LUAJIT_INCLUDE_DIRS})
find_library(LUAJIT_LIBRARY NAMES luajit-5.1 luajit HINTS ${LUAJIT_LIBDIR} ${LUAJIT_LIBRARY_DIRS})

set(LUAJIT_LIBRARIES ${LUAJIT_LIBRARY})
set(LUAJIT_INCLUDE_DIRS ${LUAJIT_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LuaJIT DEFAULT_MSG LUAJIT_LIBRARY LUAJIT_INCLUDE_DIR)
mark_as_advanced(LUAJIT_INCLUDE_DIR LUAJIT_LIBRARY)
