
add_library(rcsc_formation OBJECT
  formation.cpp
  formation_data.cpp
  formation_parser.cpp
  formation_parser_csv.cpp
  formation_parser_json.cpp
  formation_parser_static.cpp
  formation_parser_v1.cpp
  formation_parser_v2.cpp
  formation_parser_v3.cpp
  formation_dt.cpp
  formation_static.cpp
  role_type.cpp
  )

target_include_directories(rcsc_formation
  PUBLIC
  ${Boost_INCLUDE_DIRS}
  PRIVATE
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
  )

install(FILES
  formation.h
  formation_data.h
  formation_parser.h
  formation_parser_csv.h
  formation_parser_json.h
  formation_parser_static.h
  formation_parser_v1.h
  formation_parser_v2.h
  formation_parser_v3.h
  formation_dt.h
  formation_static.h
  role_type.h
  DESTINATION include/rcsc/formation
  )
