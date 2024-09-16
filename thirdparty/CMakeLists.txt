# This cmake file are managing embedded 3party Cantor dependencies

# 3dparty patched Discount
# Embedded for a while
include(ExternalProject)
set (DISCOUNT_ONLY_LIBRARY ON)
set (DISCOUNT_MAKE_INSTALL OFF)
ExternalProject_Add(
    discount_project
    URL ${CMAKE_CURRENT_SOURCE_DIR}/discount-3.0.0e-patched
    SOURCE_SUBDIR cmake
    CMAKE_ARGS -DDISCOUNT_ONLY_LIBRARY:BOOL=ON -DDISCOUNT_MAKE_INSTALL:BOOL=OFF -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
    CMAKE_CACHE_ARGS "-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    INSTALL_COMMAND ""
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/discount_project-build
    BUILD_BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/discount_project-build/${CMAKE_STATIC_LIBRARY_PREFIX}markdown${CMAKE_STATIC_LIBRARY_SUFFIX}
)

ExternalProject_Get_Property(discount_project source_dir)
ExternalProject_Get_Property(discount_project binary_dir)

add_library(Discount::Lib STATIC IMPORTED GLOBAL)
set_target_properties(Discount::Lib PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${source_dir}
    IMPORTED_LOCATION ${binary_dir}/${CMAKE_STATIC_LIBRARY_PREFIX}markdown${CMAKE_STATIC_LIBRARY_SUFFIX}
    POSITION_INDEPENDENT_CODE ON
)
add_dependencies(Discount::Lib discount_project-build)

# preview.sty
install(FILES preview.sty DESTINATION  ${KDE_INSTALL_DATADIR}/cantor/latex)
