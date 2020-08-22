
add_library(rcsc_geom_triangle OBJECT
  triangle.c
  )


add_definitions(-DTRILIBRARY -DREDUCED -DCDT_ONLY -DVOID=int -DREAL=double)

target_include_directories(rcsc_geom_triangle
  PRIVATE
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
  )
