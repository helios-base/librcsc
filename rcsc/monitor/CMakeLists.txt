
add_library(rcsc_monitor OBJECT
  monitor_command.cpp
  )

target_link_libraries(rcsc_monitor PRIVATE rcsc_build_settings)

install(FILES
  monitor_command.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rcsc/monitor
  )
