add_subdirectory(null)
add_subdirectory(sage)
add_subdirectory(maxima)

macro_optional_find_package(R)
if(R_H)
macro_optional_add_subdirectory(R)
endif(R_H)
