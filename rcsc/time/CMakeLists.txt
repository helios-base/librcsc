
add_library(rcsc_time OBJECT
  timer.cpp
  )

target_include_directories(rcsc_time
  PUBLIC
  ${Boost_INCLUDE_DIRS}
  PRIVATE
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
  )

install(FILES
  timer.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rcsc/time
  )
