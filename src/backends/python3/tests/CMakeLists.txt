add_executable(testpython3 testpython3.cpp)
target_link_libraries(testpython3 ${QT_QTTEST_LIBRARY} cantorlibs cantortest)

add_test(NAME testpython3 COMMAND testpython3)
