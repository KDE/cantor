# - Try to find the libspectre PS library
# Once done this will define
#
#  LIBSPECTRE_FOUND - system has libspectre
#  LIBSPECTRE_INCLUDE_DIR - the libspectre include directory
#  LIBSPECTRE_LIBRARY - Link this to use libspectre
#

# Copyright (c) 2006-2007, Pino Toscano, <pino@kde.org>
# Copyright (c) 2008, Albert Astals Cid, <aacid@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(LIBSPECTRE_INCLUDE_DIR AND LIBSPECTRE_LIBRARY)

  # in cache already
  set(LIBSPECTRE_INTERNAL_FOUND TRUE)

else(LIBSPECTRE_INCLUDE_DIR AND LIBSPECTRE_LIBRARY)

if(NOT WIN32)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  include(FindPkgConfig)

  if(LIBSPECTRE_MINIMUM_VERSION})
    pkg_check_modules(_pc_LIBSPECTRE libspectre>=${LIBSPECTRE_MINIMUM_VERSION})
  else(LIBSPECTRE_MINIMUM_VERSION})
    pkg_check_modules(_pc_LIBSPECTRE libspectre)
  endif(LIBSPECTRE_MINIMUM_VERSION})
else(NOT WIN32)
  # do not use pkg-config on windows
  set(_pc_LIBSPECTRE_FOUND TRUE)
endif(NOT WIN32)


if(_pc_LIBSPECTRE_FOUND)
  find_library(LIBSPECTRE_LIBRARY
    NAMES libspectre spectre
    HINTS ${_pc_LIBSPECTRE_LIBRARY_DIRS} ${CMAKE_LIBRARY_PATH}
  )

  find_path(LIBSPECTRE_INCLUDE_DIR spectre.h
    HINTS ${_pc_LIBSPECTRE_INCLUDE_DIRS}
    PATH_SUFFIXES libspectre
  )

  set(LIBSPECTRE_INTERNAL_FOUND TRUE)
endif(_pc_LIBSPECTRE_FOUND)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibSpectre DEFAULT_MSG LIBSPECTRE_LIBRARY LIBSPECTRE_INTERNAL_FOUND)

# ensure that they are cached
set(LIBSPECTRE_INCLUDE_DIR ${LIBSPECTRE_INCLUDE_DIR} CACHE INTERNAL "The libspectre include path")
set(LIBSPECTRE_LIBRARY ${LIBSPECTRE_LIBRARY} CACHE INTERNAL "The libspectre library")

endif(LIBSPECTRE_INCLUDE_DIR AND LIBSPECTRE_LIBRARY)
