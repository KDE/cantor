include_directories( ../src/lib )

kde4_add_unit_test( testsage testsage.cpp)
 
target_link_libraries( testsage
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
  cantorlibs
)
