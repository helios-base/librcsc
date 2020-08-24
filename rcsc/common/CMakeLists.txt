
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

target_include_directories(rcsc_common
  PUBLIC
  ${Boost_INCLUDE_DIRS}
  PRIVATE
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
  )

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
