add_subdirectory(null)
add_subdirectory(maxima)
add_subdirectory(octave)

if(BUILD_kalgebra)
	add_subdirectory(kalgebra)
endif(BUILD_kalgebra)

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
