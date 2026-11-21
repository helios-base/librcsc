
add_library(rcsc_trainer OBJECT
  trainer_agent.cpp
  trainer_command.cpp
  trainer_config.cpp
  )

target_link_libraries(rcsc_trainer PRIVATE rcsc_build_settings)

install(FILES
  trainer_agent.h
  trainer_command.h
  trainer_config.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rcsc/trainer
  )
