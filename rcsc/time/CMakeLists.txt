
add_library(rcsc_time OBJECT
  timer.cpp
  )

target_link_libraries(rcsc_time PRIVATE rcsc_build_settings)

install(FILES
  time_stamp.h
  timer.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rcsc/time
  )
