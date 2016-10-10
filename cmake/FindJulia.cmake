if(Julia_FOUND)
    return()
endif()

# Looking for Julia executable
find_program(Julia_EXECUTABLE julia DOC "Julia executable")
if(NOT Julia_EXECUTABLE)
    return()
endif()

# Getting Julia version
execute_process(
    COMMAND ${Julia_EXECUTABLE} --version
    OUTPUT_VARIABLE Julia_VERSION_STRING
    RESULT_VARIABLE RETURN_CODE
)
if(RETURN_CODE EQUAL 0)
    string(
        REGEX REPLACE ".*([0-9]+\\.[0-9]+\\.[0-9]+).*" "\\1"
        Julia_VERSION_STRING ${Julia_VERSION_STRING}
    )
else()
    return()
endif()

# Julia includes
execute_process(
    COMMAND ${Julia_EXECUTABLE} -E "JULIA_HOME"
    OUTPUT_VARIABLE Julia_INCLUDE_DIRS
    RESULT_VARIABLE RETURN_CODE
)
if(RETURN_CODE EQUAL 0)
    set(
        Julia_INCLUDE_DIRS
        "${Julia_INCLUDE_DIRS}/../include/julia"
    )
    string(REGEX REPLACE "(\"|\n)" "" Julia_INCLUDE_DIRS ${Julia_INCLUDE_DIRS})
    string(STRIP Julia_INCLUDE_DIRS ${Julia_INCLUDE_DIRS})
    set(
        Julia_INCLUDE_DIRS ${Julia_INCLUDE_DIRS}
        CACHE PATH "Location of Julia include files"
    )
else()
    return()
endif()

# Checking existance of main header. Some distos provide packages without actual includes
find_path(Julia_MAIN_HEADER julia.h HINTS ${Julia_INCLUDE_DIRS})
find_library(Julia_LIBRARY julia HINTS ${Julia_INCLUDE_DIRS}/../../lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Julia
    REQUIRED_VARS Julia_EXECUTABLE Julia_MAIN_HEADER Julia_INCLUDE_DIRS
    VERSION_VAR Julia_VERSION_STRING
    FAIL_MESSAGE "Julia not found"
)
