project(cantor)

macro_optional_find_package(LibSpectre)
macro_log_feature(LIBSPECTRE_FOUND "libspectre" "A PostScript rendering library" "http://libspectre.freedesktop.org/wiki/" FALSE "${LIBSPECTRE_MINIMUM_VERSION}" "Support for rendering EPS files in Cantor.")

if(LIBSPECTRE_FOUND)
  set( WITH_EPS On )
else(LIBSPECTRE_FOUND)
  set( WITH_EPS Off )
endif(LIBSPECTRE_FOUND)

include_directories( ${KDE4_INCLUDES} ${QT_INCLUDES} ${LIBSPECTRE_INCLUDE_DIR})

add_subdirectory( src )
add_subdirectory( icons )
add_subdirectory( tests )


