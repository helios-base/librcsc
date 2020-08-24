
add_library(rcsc_trainer OBJECT
  trainer_agent.cpp
  trainer_command.cpp
  trainer_config.cpp
  )

target_include_directories(rcsc_trainer
  PUBLIC
  ${Boost_INCLUDE_DIRS}
  PRIVATE
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
  )

install(FILES
  trainer_agent.h
  trainer_command.h
  trainer_config.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rcsc/trainer
  )
