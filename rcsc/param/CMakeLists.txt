
add_library(rcsc_param OBJECT
  cmd_line_parser.cpp
  conf_file_parser.cpp
  ini_parser.cpp
  param_map.cpp
  rcss_param_parser.cpp
  )

target_link_libraries(rcsc_param PRIVATE
  rcsc_build_settings
  Boost::headers # header-only
  )

install(FILES
  cmd_line_parser.h
  conf_file_parser.h
  ini_parser.h
  param_map.h
  param_parser.h
  rcss_param_parser.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rcsc/param
  )
