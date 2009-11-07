add_subdirectory(null)
add_subdirectory(maxima)

if(BUILD_kalgebra)
	add_subdirectory(kalgebra)
endif(BUILD_kalgebra)

if(NOT WIN32)
	add_subdirectory(sage)
endif(NOT WIN32)

macro_optional_find_package(R)
macro_log_feature(R_FOUND "R" "A free software environment for statistical computing and graphics" "http://www.r-project.org/" FALSE "" "Backend to use R with Cantor.")
if(R_FOUND)
	macro_optional_add_subdirectory(R)
endif(R_FOUND)
