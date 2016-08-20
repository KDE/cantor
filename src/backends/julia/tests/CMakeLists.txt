add_executable(testjulia testjulia.cpp)
target_link_libraries(testjulia ${QT_QTTEST_LIBRARY} cantorlibs cantortest)

add_test(NAME testjulia COMMAND testjulia)
