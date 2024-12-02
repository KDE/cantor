if(JULIA_FOUND)
    return()
endif()

# Find julia executable
find_program(JULIA_EXECUTABLE julia DOC "Julia executable")

if(NOT JULIA_EXECUTABLE)
    return()
endif()




#
# Julia version
#
execute_process(
    COMMAND ${JULIA_EXECUTABLE} --version
    OUTPUT_VARIABLE JULIA_VERSION_STRING
    RESULT_VARIABLE RESULT
)
if(RESULT EQUAL 0)
  string(REGEX REPLACE ".*([0-9]+\\.[0-9]+\\.[0-9]+).*" "\\1"
      JULIA_VERSION_STRING ${JULIA_VERSION_STRING})
endif()




#
# Julia includes
#
if(${JULIA_VERSION_STRING} VERSION_LESS 0.7.0)
    set(JULIA_BINDIR "JULIA_HOME")
else()
    set(JULIA_BINDIR "Sys.BINDIR")
endif()

execute_process(
    COMMAND ${JULIA_EXECUTABLE} -E "joinpath(match(r\"(.*)(bin)\",${JULIA_BINDIR}).captures[1],\"include\",\"julia\")"
    OUTPUT_VARIABLE JULIA_INCLUDE_DIRS
    # COMMAND ${JULIA_EXECUTABLE} -E "abspath(joinpath(JULIA_HOME, \"../..\", \"src\"))"
    # OUTPUT_VARIABLE JULIA_INCLUDE_DIRS
    RESULT_VARIABLE RESULT
)
if(RESULT EQUAL 0)
    string(REGEX REPLACE "\"" "" JULIA_INCLUDE_DIRS ${JULIA_INCLUDE_DIRS})
    string(REGEX REPLACE "\n" "" JULIA_INCLUDE_DIRS ${JULIA_INCLUDE_DIRS})
    set(JULIA_INCLUDE_DIRS ${JULIA_INCLUDE_DIRS}
        CACHE PATH "Location of Julia include files")
endif()

string(CONCAT JULIA_INCLUDE_TEST_FILE ${JULIA_INCLUDE_DIRS} "/julia_version.h")
string(REGEX REPLACE "\n" "" JULIA_INCLUDE_TEST_FILE ${JULIA_INCLUDE_TEST_FILE})
IF (NOT EXISTS ${JULIA_INCLUDE_TEST_FILE})
    message(STATUS "Julia found, but include files are missing")
    return()
endif()


#
# Julia library location
#
if(${JULIA_VERSION_STRING} VERSION_LESS 0.7.0)
    set(JULIA_LIBDL_COMMAND "abspath(dirname(Libdl.dlpath(\"libjulia\")))")
else()
    set(JULIA_LIBDL_COMMAND "using Libdl\; abspath(dirname(Libdl.dlpath(\"libjulia\")))")
endif()

execute_process(
    COMMAND ${JULIA_EXECUTABLE} -E ${JULIA_LIBDL_COMMAND}
    OUTPUT_VARIABLE JULIA_LIBRARY_DIR
    RESULT_VARIABLE RESULT
)

if(RESULT EQUAL 0)
    string(REGEX REPLACE "\"" "" JULIA_LIBRARY_DIR ${JULIA_LIBRARY_DIR})
    string(STRIP ${JULIA_LIBRARY_DIR} JULIA_LIBRARY_DIR)
    if(WIN32) # static libs are in lib
        string(REGEX REPLACE "bin$" "lib" JULIA_LIBRARY_DIR ${JULIA_LIBRARY_DIR})
    endif()
    set(JULIA_LIBRARY_DIR ${JULIA_LIBRARY_DIR}
        CACHE PATH "Julia library directory")
endif()

if(NOT ${JULIA_VERSION_STRING} VERSION_LESS 1.6.0)
    set(JULIA_INTERNAL_LIBDL_COMMAND "using Libdl\; abspath(dirname(Libdl.dlpath(\"libjulia-internal\")))")
    execute_process(
        COMMAND ${JULIA_EXECUTABLE} -E ${JULIA_INTERNAL_LIBDL_COMMAND}
        OUTPUT_VARIABLE JULIA_INTERNAL_LIBRARY_DIR
        RESULT_VARIABLE RESULT
    )

    if(RESULT EQUAL 0)
        string(REGEX REPLACE "\"" "" JULIA_INTERNAL_LIBRARY_DIR ${JULIA_INTERNAL_LIBRARY_DIR})
        string(STRIP ${JULIA_INTERNAL_LIBRARY_DIR} JULIA_INTERNAL_LIBRARY_DIR)
        if(WIN32) # static libs are in lib
            string(REGEX REPLACE "bin$" "lib" JULIA_INTERNAL_LIBRARY_DIR ${JULIA_INTERNAL_LIBRARY_DIR})
        endif()
        set(JULIA_INTERNAL_LIBRARY_DIR ${JULIA_INTERNAL_LIBRARY_DIR}
            CACHE PATH "Julia internal library directory")
    endif()
endif()

# linking to .dll files is not allowed
set(CMAKE_FIND_LIBRARY_SUFFIXES .so .a .lib)

find_library( JULIA_LIBRARY
    # static library name on Windows is libjulia.dll.a
    NAMES julia julia.dll
    PATHS ${JULIA_LIBRARY_DIR}
)

if(NOT ${JULIA_VERSION_STRING} VERSION_LESS 1.6.0)
    find_library( JULIA_INTERNAL_LIBRARY
	# static library name on Windows is libjulia-internal.dll.a
        NAMES julia-internal julia-internal.dll
        PATHS ${JULIA_INTERNAL_LIBRARY_DIR}
    )
endif()

if(${JULIA_VERSION_STRING} VERSION_LESS 1.6.0)
    find_package_handle_standard_args(
        Julia
        REQUIRED_VARS   JULIA_LIBRARY JULIA_INCLUDE_DIRS
        VERSION_VAR     JULIA_VERSION_STRING
        FAIL_MESSAGE    "Julia not found"
    )
else()
    find_package_handle_standard_args(
        Julia
        REQUIRED_VARS   JULIA_LIBRARY JULIA_INTERNAL_LIBRARY JULIA_INCLUDE_DIRS
        VERSION_VAR     JULIA_VERSION_STRING
        FAIL_MESSAGE    "Julia not found"
    )
endif()
