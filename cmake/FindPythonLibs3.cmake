find_package(PkgConfig)
pkg_check_modules(PYTHONLIBS3 QUIET python3)
set(PYTHONLIBS3_DEFINITIONS ${PYTHONLIBS3_CFLAGS_OTHER})

find_path(PYTHONLIBS3_INCLUDE_DIR Python.h
          HINTS ${PYTHONLIBS3_INCLUDEDIR} ${PYTHONLIBS3_INCLUDE_DIRS})

IF(WIN32)
  find_library(PYTHONLIBS3_LIBRARY NAMES python32 python33 python34 python35 python36 python37 python38
               HINTS ${PYTHONLIBS3_LIBDIR} ${PYTHONLIBS3_LIBRARY_DIRS} )
ELSE()
  find_library(PYTHONLIBS3_LIBRARY NAMES python3.2m python3.3m python3.4m python3.5m python3.6m python3.7m python3.8
               HINTS ${PYTHONLIBS3_LIBDIR} ${PYTHONLIBS3_LIBRARY_DIRS} )
ENDIF()

set(PYTHONLIBS3_LIBRARIES ${PYTHONLIBS3_LIBRARY} )
set(PYTHONLIBS3_INCLUDE_DIRS ${PYTHONLIBS3_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PythonLibs3  DEFAULT_MSG
                                  PYTHONLIBS3_LIBRARY PYTHONLIBS3_INCLUDE_DIR)

mark_as_advanced(PYTHONLIBS3_INCLUDE_DIR PYTHONLIBS3_LIBRARY)
