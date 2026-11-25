
add_library(rcsc_gz OBJECT
  gzcompressor.cpp
  gzfstream.cpp
  gzfilterstream.cpp
  gzread_file.cpp
  )

target_link_libraries(rcsc_gz PRIVATE rcsc_build_settings)
if(ZLIB_FOUND)
  target_link_libraries(rcsc_gz PRIVATE ZLIB::ZLIB)
endif()

install(FILES
  gzcompressor.h
  gzfstream.h
  gzfilterstream.h
  gzread_file.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rcsc/gz
  )

# unit tests (CppUnit): the same program as "make check" of the Autotools build
if(LIBRCSC_UNIT_TESTS)
  add_executable(run_test_gzread_file test_gzread_file.cpp)
  target_link_libraries(run_test_gzread_file PRIVATE rcsc_build_settings rcsc PkgConfig::CPPUNIT)
  add_test(NAME run_test_gzread_file COMMAND run_test_gzread_file)
  librcsc_set_unit_test_environment(run_test_gzread_file)
endif()
