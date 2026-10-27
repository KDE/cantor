include_directories(${R_INCLUDE_DIR})

set( RServer_SRCS
   rserver.cpp
   rcallbacks.cpp
   main.cpp
)

kconfig_add_kcfg_files(RServer_SRCS settings.kcfgc)
install(FILES rserver.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

add_custom_command(OUTPUT renvvars.h COMMAND ${R_EXECUTABLE} --slave --file=${CMAKE_CURRENT_SOURCE_DIR}/tools/envvars.r > ${CMAKE_CURRENT_BINARY_DIR}/renvvars.h)
add_custom_target(renvvars DEPENDS renvvars.h)

add_custom_command(OUTPUT rautoloads.h COMMAND ${R_EXECUTABLE} --slave --file=${CMAKE_CURRENT_SOURCE_DIR}/tools/autoloads.r > ${CMAKE_CURRENT_BINARY_DIR}/rautoloads.h )
add_custom_target(rautoloads DEPENDS rautoloads.h)

add_definitions(-DENABLE_LEGACY_NONAPI)

add_executable( cantor_rserver ${RServer_SRCS} )
set_target_properties( cantor_rserver PROPERTIES INSTALL_RPATH_USE_LINK_PATH false)
target_link_libraries( cantor_rserver cantorlibs ${R_LIBRARIES} Qt6::Network)
add_dependencies(cantor_rserver renvvars rautoloads)

# Windows-specific: 64MB stack size (same as R on Windows)
IF(WIN32)
    IF(MSVC)
        SET_TARGET_PROPERTIES(cantor_rserver PROPERTIES LINK_FLAGS /STACK:64000000)
    ELSE(MSVC)
        SET_TARGET_PROPERTIES(cantor_rserver PROPERTIES LINK_FLAGS -Wl,--stack,64000000)
    ENDIF(MSVC)
ENDIF(WIN32)

# macOS-specific: CoreFoundation framework and code signing
IF(APPLE)
    TARGET_LINK_LIBRARIES(cantor_rserver "-framework CoreFoundation")
    # Add entitlements for loading external R library and JIT
    # ADD_CUSTOM_COMMAND(TARGET cantor_rserver POST_BUILD
    #     COMMAND codesign -d --force --options runtime --verbose=4 -s -
    #         --entitlements "${CMAKE_CURRENT_SOURCE_DIR}/cantor_rserver.entitlements"
    #         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/cantor_rserver
    #     WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    #     COMMENT "Adding entitlements for cantor_rserver to link to external R lib...")
ENDIF(APPLE)

IF(MSVC)
# When compiling with MSVC, we have to create a .lib file for R.dll, first
FIND_PACKAGE(Python3 COMPONENTS Interpreter REQUIRED)
set(R_IMPORT_LIB ${CMAKE_BINARY_DIR}/R.lib)
ADD_CUSTOM_COMMAND(OUTPUT ${R_IMPORT_LIB}
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../GenLibFile.py ${R_R_LIBRARY} ${CMAKE_BINARY_DIR} ${CMAKE_SYSTEM_PROCESSOR}
    COMMENT "Generate .lib file for R.dll (rserver)")
ADD_CUSTOM_TARGET(R_lib_rserver DEPENDS ${R_IMPORT_LIB})
ADD_DEPENDENCIES(cantor_rserver R_lib_rserver)
# Replace DLL with import lib in link libraries
set_property(TARGET cantor_rserver PROPERTY LINK_LIBRARIES cantorlibs ${R_IMPORT_LIB} Qt6::Network)
ENDIF(MSVC)

install(TARGETS cantor_rserver ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
