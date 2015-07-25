cmake_minimum_required(VERSION 2.8.9)
project(cantor)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)

set(CMAKE_MODULE_PATH ${cantor_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

macro_optional_find_package(Analitza)
macro_log_feature(Analitza_FOUND "libanalitza" "A library provided by KAlgebra" FALSE "" "Backend to use KAlgebra with Cantor.")

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
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)


add_subdirectory( doc )
add_subdirectory( src )
add_subdirectory( icons )

if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
   macro_display_feature_log()
endif (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})

install( FILES cantor.appdata.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata/ )

