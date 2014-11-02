add_subdirectory(null)
# add_subdirectory(maxima)
add_subdirectory(octave)
# add_subdirectory(scilab)

# find_package(Analitza)
# set_package_properties(Analitza PROPERTIES DESCRIPTION "A library provided by KAlgebra"
#                        URL "http://edu.kde.org/kalgebra/"
#                        TYPE OPTIONAL
#                        PURPOSE "Backend to use KAlgebra with Cantor")
# if(Analitza_FOUND)
#   add_subdirectory(kalgebra)
# endif(Analitza_FOUND)

# if(NOT WIN32)
#   add_subdirectory(sage)
# endif(NOT WIN32)

# find_package(R)
# set_package_properties(R PROPERTIES DESCRIPTION "A free software environment for statistical computing and graphics"
#                        URL "http://www.r-project.org/"
#                        TYPE OPTIONAL
#                        PURPOSE "Backend to use R with Cantor")
#
# if(R_FOUND)
#   add_subdirectory(R)
# endif(R_FOUND)

# macro_optional_find_package(Qalculate)
# macro_log_feature(QALCULATE_FOUND "Qalculate" "A multi-purpose desktop calculator with support for customizable functions, units and arbitrary precision." "http://qalculate.sourceforge.net/" FALSE "" "Backend to use Qalculate with Cantor.")
# if(QALCULATE_FOUND)
#     ecm_optional_add_subdirectory(qalculate)
# endif(QALCULATE_FOUND)

find_package(PythonLibs 2.7)
set_package_properties(PythonLibs PROPERTIES DESCRIPTION "A powerful dynamic programming language"
    URL "https://www.python.org/"
    TYPE OPTIONAL
    PURPOSE "Backend to use Python 2 with Cantor.")

if(PYTHONLIBS_FOUND)
    add_subdirectory(python2)
endif(PYTHONLIBS_FOUND)

# macro_optional_find_package(LuaJIT)
# macro_log_feature(LUAJIT_FOUND "Lua" "A lightweight, extensible programming language (luajit implementation)." "http://www.lua.org/" FALSE "" "Backend to use Lua (luajit2) with Cantor")
# if(LUAJIT_FOUND)
#     add_subdirectory(lua)
# endif(LUAJIT_FOUND)