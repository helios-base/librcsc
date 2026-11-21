
add_library(rcsc_param OBJECT
  cmd_line_parser.cpp
  conf_file_parser.cpp
  ini_parser.cpp
  param_map.cpp
  rcss_param_parser.cpp
  )

target_include_directories(rcsc_param
  PRIVATE
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
  )

# header-only
target_link_libraries(rcsc_param PRIVATE Boost::headers)

install(FILES
  cmd_line_parser.h
  conf_file_parser.h
  ini_parser.h
  param_map.h
  param_parser.h
  rcss_param_parser.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rcsc/param
  )
