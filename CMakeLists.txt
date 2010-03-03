project(cantor)

find_package(LibAttica REQUIRED)

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
add_subdirectory( tests )


