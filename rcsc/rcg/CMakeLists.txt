
add_library(rcsc_rcg OBJECT
  simdjson/simdjson.cpp
  handler.cpp
  parser.cpp
  parser_v1.cpp
  parser_v2.cpp
  parser_v3.cpp
  parser_v4.cpp
  parser_simdjson.cpp
  serializer.cpp
  serializer_v1.cpp
  serializer_v2.cpp
  serializer_v3.cpp
  serializer_v4.cpp
  serializer_v5.cpp
  serializer_v6.cpp
  serializer_json.cpp
  types.cpp
  util.cpp
  )

target_link_libraries(rcsc_rcg PRIVATE rcsc_build_settings)

install(FILES
  handler.h
  parser.h
  parser_v1.h
  parser_v2.h
  parser_v3.h
  parser_v4.h
  parser_simdjson.h
  serializer.h
  serializer_v1.h
  serializer_v2.h
  serializer_v3.h
  serializer_v4.h
  serializer_v5.h
  serializer_v6.h
  serializer_json.h
  types.h
  util.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rcsc/rcg
  )

# unit tests (CppUnit): the same program as "make check" of the Autotools build
if(LIBRCSC_UNIT_TESTS)
  add_executable(run_test_rcg_types test_types.cpp)
  target_link_libraries(run_test_rcg_types PRIVATE rcsc_build_settings rcsc PkgConfig::CPPUNIT)
  add_test(NAME run_test_rcg_types COMMAND run_test_rcg_types)
  librcsc_set_unit_test_environment(run_test_rcg_types)

  # the test data are the first records of real game logs (test_data/)
  add_executable(run_test_rcg_parser test_parser.cpp)
  target_link_libraries(run_test_rcg_parser PRIVATE rcsc_build_settings rcsc PkgConfig::CPPUNIT)
  target_compile_definitions(run_test_rcg_parser PRIVATE
    TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/test_data")
  add_test(NAME run_test_rcg_parser COMMAND run_test_rcg_parser)
  librcsc_set_unit_test_environment(run_test_rcg_parser)
endif()
