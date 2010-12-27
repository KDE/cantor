 # - Try to find the Cantor Library
# Once done this will define
#
#  CANTOR_FOUND - system has Cantor
#  CANTOR_INCLUDE_DIR - the Cantor include directory
#  CANTOR_LIBRARIES 
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if ( CANTOR_INCLUDE_DIR AND CANTOR_LIBRARIES )
   # in cache already
   SET( CANTOR_FIND_QUIETLY TRUE )
endif ( CANTOR_INCLUDE_DIR AND CANTOR_LIBRARIES )

FIND_PATH( CANTOR_INCLUDE_DIR NAMES backend.h PATH_SUFFIXES cantor
)

FIND_LIBRARY( CANTOR_LIBRARIES NAMES cantorlibs )


include( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( cantor DEFAULT_MSG CANTOR_INCLUDE_DIR CANTOR_LIBRARIES )
