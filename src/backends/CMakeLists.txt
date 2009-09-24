add_subdirectory(null)
add_subdirectory(sage)
add_subdirectory(maxima)

macro_optional_find_package(R)
macro_log_feature(R_H "R" "A free software environment for statistical computing and graphics" "http://www.r-project.org/" FALSE "" "Backend to use R with Cantor.")
if(R_H)
macro_optional_add_subdirectory(R)
endif(R_H)
