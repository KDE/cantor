# - Try to find R
# Once done this will define
#
#  R_FOUND - system has R
#  R_EXECUTABLE - executable of R
#  R_HOME - home directory of R
#  R_INCLUDE_DIR - the R include directory
#  R_LIBRARIES - Link these to use R

# find the R binary
FIND_PROGRAM(R_EXECUTABLE R)

IF(R_EXECUTABLE)

  # find R_HOME
  IF(NOT R_HOME)
    EXECUTE_PROCESS(
      COMMAND ${R_EXECUTABLE} "--slave" "--no-save" "-e" "cat(R.home())"
      OUTPUT_VARIABLE R_HOME)
  ENDIF(NOT R_HOME)
  IF(NOT R_HOME)
    MESSAGE(STATUS "Could NOT determine R_HOME (probably you misspecified the location of R)")
  ENDIF(NOT R_HOME)

  # find R include dir
  IF(NOT R_INCLUDE_DIR)
    IF(WIN32)    # This version of the test will not work with R < 2.9.0, but the other version (in the else part) will not work on windows (and on windows the paths are generally standard, anyway).
      EXECUTE_PROCESS(
        COMMAND ${R_EXECUTABLE} "--slave" "--no-save" "-e" "cat(R.home('include'))"
        OUTPUT_VARIABLE R_INCLUDE_DIR)
    ELSE(WIN32)
      EXECUTE_PROCESS(
        COMMAND ${R_EXECUTABLE} CMD sh -c "echo -n $R_INCLUDE_DIR"
        OUTPUT_VARIABLE R_INCLUDE_DIR)
    ENDIF(WIN32)
  ENDIF(NOT R_INCLUDE_DIR)

  IF(NOT R_INCLUDE_DIR)
    SET(R_INCLUDE_DIR ${R_HOME}/include)
    MESSAGE(STATUS "R_Home not findable via R. Guessing")
  ENDIF(NOT R_INCLUDE_DIR)

  FIND_PATH(R_INCLUDE_DIR R.h)

  # check for existence of libR.so

  FIND_LIBRARY(R_R_LIBRARY
    R
    HINTS ${R_HOME}/lib ${R_SHARED_LIB_DIR} ${R_HOME}/bin )
  IF(NOT R_R_LIBRARY)
    MESSAGE(STATUS "libR not found. Make sure the location of R was detected correctly, above, and R was compiled with the --enable-shlib option")
  ELSE(NOT R_R_LIBRARY)
    GET_FILENAME_COMPONENT(R_SHARED_LIB_DIR ${R_R_LIBRARY}
      PATH)
    SET(R_LIBRARIES ${R_R_LIBRARY})
  ENDIF(NOT R_R_LIBRARY)

  # for at least some versions of R, we seem to have to link against -lRlapack. Else loading some
  # R packages will fail due to unresolved symbols, or we can't link against -lR.
  # However, we can't do this unconditionally,
  # as this is not available in some configurations of R

  FIND_LIBRARY(R_LAPACK_LIBRARY
    Rlapack
    HINTS ${R_SHARED_LIB_DIR} )
  IF(NOT R_LAPACK_LIBRARY)
    #MESSAGE(STATUS "No, it does not exist in ${R_SHARED_LIB_DIR}")
  ELSE(NOT R_LAPACK_LIBRARY)
    #MESSAGE(STATUS "Yes, ${R_LAPACK_LIBRARY} exists")
    SET(R_LIBRARIES ${R_LIBRARIES} ${R_LAPACK_LIBRARY})
    IF(NOT WIN32)
      FIND_LIBRARY(GFORTRAN_LIBRARY
        gfortran)
      IF (GFORTRAN_LIBRARY)
        # needed when linking to Rlapack on linux for some unknown reason.
        # apparently not needed on windows (let's see, when it comes back to bite us, though)
        # and compiling on windows is hard enough even without requiring libgfortran, too.
        SET(R_LIBRARIES ${R_LIBRARIES} gfortran)
      ELSE (GFORTRAN_LIBRARY)
        SET(R_FOUND false)
      ENDIF (GFORTRAN_LIBRARY)
    ENDIF(NOT WIN32)
  ENDIF(NOT R_LAPACK_LIBRARY)

  # for at least some versions of R, we seem to have to link against -lRlapack. Else loading some
  # R packages will fail due to unresolved symbols, or we can't link against -lR.
  # However, we can't do this unconditionally,
  # as this is not available in some configurations of R

  FIND_LIBRARY(R_BLAS_LIBRARY
    Rblas
    HINTS ${R_SHARED_LIB_DIR} )
  IF(NOT R_BLAS_LIBRARY)
    #MESSAGE(STATUS "No, it does not exist in ${R_SHARED_LIB_DIR}")
  ELSE(NOT R_BLAS_LIBRARY)
    #MESSAGE(STATUS "Yes, ${R_BLAS_LIBRARY} exists")
    SET(R_LIBRARIES ${R_LIBRARIES} ${R_BLAS_LIBRARY})
  ENDIF(NOT R_BLAS_LIBRARY)

ENDIF( R_EXECUTABLE )

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(R  DEFAULT_MSG
                                  R_EXECUTABLE R_INCLUDE_DIR R_R_LIBRARY)

MARK_AS_ADVANCED(R_INCLUDE_DIR R_R_LIBRARY R_LAPACK_LIBRARY R_BLAS_LIBRARY)
