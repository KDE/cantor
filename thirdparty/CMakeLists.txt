# This cmake file are managing embeded 3party Cantor dependencies

# 3dparty patched Discount
# Embedded for a while
# TODO: There is CMakeLists.txt in Discount library (cmake subfolder)
# Maybe better use this file instead of working with configure?
file(COPY discount-2.2.6-patched/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/discount)

message(STATUS "Configure embedded patched 3dparty Discount library")
# Run configure and create Makefile
execute_process(
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/discount/configure.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/discount
    OUTPUT_QUIET
)


set(DiscountLib_SRC
    ${CMAKE_CURRENT_BINARY_DIR}/discount/mkdio.c
    ${CMAKE_CURRENT_BINARY_DIR}/discount/markdown.c
    ${CMAKE_CURRENT_BINARY_DIR}/discount/dumptree.c
    ${CMAKE_CURRENT_BINARY_DIR}/discount/generate.c
    ${CMAKE_CURRENT_BINARY_DIR}/discount/resource.c
    ${CMAKE_CURRENT_BINARY_DIR}/discount/docheader.c
    ${CMAKE_CURRENT_BINARY_DIR}/discount/version.c
    ${CMAKE_CURRENT_BINARY_DIR}/discount/toc.c
    ${CMAKE_CURRENT_BINARY_DIR}/discount/css.c
    ${CMAKE_CURRENT_BINARY_DIR}/discount/xml.c
    ${CMAKE_CURRENT_BINARY_DIR}/discount/Csio.c
    ${CMAKE_CURRENT_BINARY_DIR}/discount/xmlpage.c
    ${CMAKE_CURRENT_BINARY_DIR}/discount/basename.c
    ${CMAKE_CURRENT_BINARY_DIR}/discount/emmatch.c
    ${CMAKE_CURRENT_BINARY_DIR}/discount/github_flavoured.c
    ${CMAKE_CURRENT_BINARY_DIR}/discount/setup.c
    ${CMAKE_CURRENT_BINARY_DIR}/discount/tags.c
    ${CMAKE_CURRENT_BINARY_DIR}/discount/html5.c
    ${CMAKE_CURRENT_BINARY_DIR}/discount/flags.c
)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/discount/libmarkdown
                   COMMAND make
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/discount
                   DEPENDS ${DiscountLib_SRC}
)
add_custom_target(Discount ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/discount/libmarkdown)

set(Discount_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/discount PARENT_SCOPE)
set(Discount_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/discount/libmarkdown PARENT_SCOPE)
