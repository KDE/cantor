# This cmake file are managing embedded 3party Cantor dependencies

# 3dparty patched Discount
# Embedded for a while
include(ExternalProject)
set (DISCOUNT_ONLY_LIBRARY ON)
set (DISCOUNT_MAKE_INSTALL OFF)
ExternalProject_Add(
    discount_project
    URL ${CMAKE_CURRENT_SOURCE_DIR}/discount-2.2.6-patched
    SOURCE_SUBDIR cmake
    CMAKE_ARGS DISCOUNT_ONLY_LIBRARY DISCOUNT_MAKE_INSTALL
    CMAKE_CACHE_ARGS "-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    INSTALL_COMMAND ""
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/discount_project-build
    BUILD_BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/discount_project-build/libmarkdown${CMAKE_STATIC_LIBRARY_SUFFIX}
)

ExternalProject_Get_Property(discount_project source_dir)
ExternalProject_Get_Property(discount_project binary_dir)

add_library(Discount::Lib STATIC IMPORTED GLOBAL)
set_target_properties(Discount::Lib PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${source_dir}
    IMPORTED_LOCATION ${binary_dir}/libmarkdown${CMAKE_STATIC_LIBRARY_SUFFIX}
    POSITION_INDEPENDENT_CODE ON
)
add_dependencies(Discount::Lib discount_project-build)
set(Discount_FOUND TRUE)

# preview.sty
install(FILES standalone.cls DESTINATION  ${KDE_INSTALL_DATADIR}/cantor/latex)
