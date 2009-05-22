project(mathematik)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules )

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
find_package(LibSpectre REQUIRED)

include_directories( ${KDE4_INCLUDES} ${QT_INCLUDES} ${LIBSPECTRE_INCLUDE_DIR})

add_subdirectory(cmake)
add_subdirectory( src )
add_subdirectory( icons )
