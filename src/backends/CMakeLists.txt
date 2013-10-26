add_subdirectory(null)
add_subdirectory(maxima)
add_subdirectory(octave)
add_subdirectory(scilab)

if(Analitza_FOUND)
	add_subdirectory(kalgebra)
endif(Analitza_FOUND)

if(NOT WIN32)
	add_subdirectory(sage)
endif(NOT WIN32)

if(R_FOUND)
	macro_optional_add_subdirectory(R)
endif(R_FOUND)

macro_optional_find_package(Qalculate)
macro_log_feature(QALCULATE_FOUND "Qalculate" "A multi-purpose desktop calculator with support for customizable functions, units and arbitrary precision." "http://qalculate.sourceforge.net/" FALSE "" "Backend to use Qalculate with Cantor.")
if(QALCULATE_FOUND)
    macro_optional_add_subdirectory(qalculate)
endif(QALCULATE_FOUND)

macro_optional_find_package(PythonLibs 2.7)
macro_log_feature(PYTHONLIBS_FOUND "Python" "A powerful dynamic programming language" "http://python.org/" FALSE "" "Backend to use Python with Cantor.")
if(PYTHONLIBS_FOUND)
    add_subdirectory(python2)
endif(PYTHONLIBS_FOUND)