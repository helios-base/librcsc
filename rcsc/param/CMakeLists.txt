
add_library(rcsc_param OBJECT
  cmd_line_parser.cpp
  conf_file_parser.cpp
  param_map.cpp
  rcss_param_parser.cpp
  )

target_include_directories(rcsc_param
  PUBLIC
  ${Boost_INCLUDE_DIRS}
  PRIVATE
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
  )

install(FILES
  cmd_line_parser.h
  conf_file_parser.h
  param_map.h
  param_parser.h
  rcss_param_parser.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rcsc/param
  )
