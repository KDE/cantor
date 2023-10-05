include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(cantortest_SRCS
    backendtest.cpp)

add_library( cantortest SHARED ${cantortest_SRCS} )
generate_export_header(cantortest)

target_link_libraries( cantortest
    cantorlibs
    Qt6::Test)
