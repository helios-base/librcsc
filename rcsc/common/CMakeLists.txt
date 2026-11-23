
add_library(rcsc_common OBJECT
  abstract_client.cpp
  audio_codec.cpp
  audio_memory.cpp
  logger.cpp
  offline_client.cpp
  online_client.cpp
  player_param.cpp
  player_type.cpp
  say_message_parser.cpp
  server_param.cpp
  soccer_agent.cpp
  stamina_model.cpp
  team_graphic.cpp
  )

target_link_libraries(rcsc_common PRIVATE rcsc_build_settings)

install(FILES
  abstract_client.h
  audio_codec.h
  audio_memory.h
  audio_message.h
  free_message_parser.h
  freeform_message.h
  freeform_message_parser.h
  logger.h
  offline_client.h
  online_client.h
  player_param.h
  player_type.h
  say_message.h
  say_message_parser.h
  server_param.h
  soccer_agent.h
  stamina_model.h
  team_graphic.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rcsc/common
  )

# unit tests (CppUnit): the same program as "make check" of the Autotools build
if(LIBRCSC_UNIT_TESTS)
  add_executable(run_test_server_param test_server_param.cpp)
  target_link_libraries(run_test_server_param PRIVATE rcsc_build_settings rcsc PkgConfig::CPPUNIT)
  add_test(NAME run_test_server_param COMMAND run_test_server_param)
  librcsc_set_unit_test_environment(run_test_server_param)
endif()
