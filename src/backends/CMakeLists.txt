function(add_backend name)

    kcoreaddons_add_plugin("cantor_${name}"
        SOURCES ${ARGN}
        JSON "${name}.json"
        INSTALL_NAMESPACE "cantor/backends")

    target_link_libraries("cantor_${name}"
        cantorlibs)

endfunction()

add_subdirectory(maxima)
add_subdirectory(octave)
add_subdirectory(scilab)

if(NOT WIN32)
    add_subdirectory(sage)
endif(NOT WIN32)

set_package_properties(Analitza5 PROPERTIES DESCRIPTION "A library provided by KAlgebra."
    URL "http://edu.kde.org/kalgebra/"
    TYPE OPTIONAL
    PURPOSE "Backend to use KAlgebra with Cantor.")

find_package(Analitza5)
if(Analitza5_FOUND)
    add_subdirectory(kalgebra)
endif(Analitza5_FOUND)

set_package_properties(R PROPERTIES DESCRIPTION "A free software environment for statistical computing and graphics."
    URL "http://www.r-project.org/"
    TYPE OPTIONAL
    PURPOSE "Backend to use R with Cantor.")

find_package(R)
if(R_FOUND)
    add_subdirectory(R)
endif(R_FOUND)

set_package_properties(Qalculate PROPERTIES DESCRIPTION "A multi-purpose desktop calculator with support for customizable functions, units and arbitrary precision."
    URL "http://qalculate.sourceforge.net/"
    TYPE OPTIONAL
    PURPOSE "Backend to use Qalculate with Cantor.")

find_package(Qalculate)
if(QALCULATE_FOUND)
    add_subdirectory(qalculate)
endif(QALCULATE_FOUND)

set_package_properties(PythonLibs PROPERTIES DESCRIPTION "A powerful dynamic programming language."
    URL "https://www.python.org/"
    TYPE OPTIONAL
    PURPOSE "Backend to use Python 2 with Cantor.")

# If PythonInterp has been found (which it is indirectly by KF5I18n),
# find_package(PythonLibs) will prefer that version over whatever
# version information you pass the find_package call. Set a special
# variable to override that:
set(Python_ADDITIONAL_VERSIONS 2.7)
find_package(PythonLibs 2.7)

set_package_properties(PythonLibs3 PROPERTIES DESCRIPTION "A powerful dynamic programming language."
    URL "https://www.python.org/"
    TYPE OPTIONAL
    PURPOSE "Backend to use Python 3 with Cantor.")

find_package(PythonLibs3)

if(PYTHONLIBS_FOUND OR PYTHONLIBS3_FOUND)
    add_subdirectory(python)
endif(PYTHONLIBS_FOUND OR PYTHONLIBS3_FOUND)

if(PYTHONLIBS_FOUND)
    add_subdirectory(python2)
endif(PYTHONLIBS_FOUND)

if(PYTHONLIBS3_FOUND)
    add_subdirectory(python3)
endif(PYTHONLIBS3_FOUND)

set_package_properties(LuaJIT PROPERTIES DESCRIPTION "A lightweight, extensible programming language (luajit implementation)."
    URL "http://www.lua.org/"
    TYPE OPTIONAL
    PURPOSE "Backend to use Lua (luajit2) with Cantor.")

find_package(LuaJIT)
if(LUAJIT_FOUND)
    add_subdirectory(lua)
endif(LUAJIT_FOUND)

set_package_properties(
    Julia
    PROPERTIES DESCRIPTION "A high-level, high-performance dynamic programming language for technical computing"
    URL "http://julialang.org/"
    TYPE OPTIONAL
    PURPOSE "Backend to use Julia with Cantor.")

find_package(Julia)
if(Julia_FOUND)
    add_subdirectory(julia)
endif()
