add_subdirectory(null)
add_subdirectory(maxima)
add_subdirectory(octave)

if(Analitza_FOUND)
	add_subdirectory(kalgebra)
endif(Analitza_FOUND)

if(NOT WIN32)
	add_subdirectory(sage)
endif(NOT WIN32)

if(R_FOUND)
	macro_optional_add_subdirectory(R)
endif(R_FOUND)
