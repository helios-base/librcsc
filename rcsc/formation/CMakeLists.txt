
add_library(rcsc_formation OBJECT
  formation.cpp
  sample_data.cpp
  formation_dt.cpp
  formation_static.cpp
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
  sample_data.h
  formation_dt.h
  formation_static.h
  DESTINATION include/rcsc/formation
  )
