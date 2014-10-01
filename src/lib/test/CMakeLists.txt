include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(cantortest_SRCS
    backendtest.cpp)

add_library( cantortest SHARED ${cantortest_SRCS} )

target_link_libraries( cantortest
    cantorlibs
    Qt5::Test)