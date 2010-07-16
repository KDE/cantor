project(cantor)

macro_optional_find_package(R)
macro_log_feature(R_FOUND "R" "A free software environment for statistical computing and graphics" "http://www.r-project.org/" FALSE "" "Backend to use R with Cantor.")

if(NOT WIN32)
  macro_optional_find_package(LibSpectre)
  macro_log_feature(LIBSPECTRE_FOUND "libspectre" "A PostScript rendering library" "http://libspectre.freedesktop.org/wiki/" FALSE "${LIBSPECTRE_MINIMUM_VERSION}" "Support for rendering EPS files in Cantor.")

  if(LIBSPECTRE_FOUND)
    set( WITH_EPS On )
  else(LIBSPECTRE_FOUND)
    set( WITH_EPS Off )
  endif(LIBSPECTRE_FOUND)
else(NOT WIN32)
  set( WITH_EPS Off )
endif(NOT WIN32)

include_directories( ${KDE4_INCLUDES} ${QT_INCLUDES} )

add_subdirectory( src )
add_subdirectory( icons )


