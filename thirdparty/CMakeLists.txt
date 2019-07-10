# This cmake file are managing embeded 3party Cantor dependencies

# 3dparty patched Discount
# Embedded for a while
file(COPY discount-2.2.6-patched/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/discount)

message(STATUS "Configure embedded patched 3dparty Discount library")
# Run configure and create Makefile
execute_process(
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/discount/configure.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/discount
    OUTPUT_QUIET
)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/discount.stash
                   COMMAND make
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/discount
)
add_custom_target(Discount ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/discount.stash)

set(Discount_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/discount PARENT_SCOPE)
set(Discount_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/discount/libmarkdown PARENT_SCOPE)
