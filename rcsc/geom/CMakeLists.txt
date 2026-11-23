
add_library(rcsc_geom OBJECT
  angle_deg.cpp
  circle_2d.cpp
  composite_region_2d.cpp
  constrained_delaunay_triangulation.cpp
  convex_hull.cpp
  delaunay_triangulation.cpp
  delaunay_triangulation_core.cpp
  line_2d.cpp
  matrix_2d.cpp
  polygon_2d.cpp
  ray_2d.cpp
  rect_2d.cpp
  sector_2d.cpp
  segment_2d.cpp
  triangle_2d.cpp
  vector_2d.cpp
  voronoi_diagram.cpp
  )

#set_property(TARGET rcsc_geom PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(rcsc_geom PRIVATE rcsc_build_settings)

install(FILES
  angle_deg.h
  circle_2d.h
  composite_region_2d.h
  constrained_delaunay_triangulation.h
  convex_hull.h
  delaunay_triangulation.h
  delaunay_triangulation_core.h
  line_2d.h
  matrix_2d.h
  polygon_2d.h
  ray_2d.h
  rect_2d.h
  region_2d.h
  sector_2d.h
  size_2d.h
  segment_2d.h
  triangle_2d.h
  vector_2d.h
  voronoi_diagram.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rcsc/geom
  )

# unit tests (CppUnit): the same programs as "make check" of the Autotools build
if(LIBRCSC_UNIT_TESTS)
  foreach(name vector_2d matrix_2d segment_2d triangle_2d rect_2d polygon_2d voronoi_diagram convex_hull)
    add_executable(run_test_${name} test_${name}.cpp)
    target_link_libraries(run_test_${name} PRIVATE rcsc_build_settings rcsc PkgConfig::CPPUNIT)
    add_test(NAME run_test_${name} COMMAND run_test_${name})
    librcsc_set_unit_test_environment(run_test_${name})
  endforeach()
  add_executable(rundom_convex_hull test_rundom_convex_hull.cpp)
  target_link_libraries(rundom_convex_hull PRIVATE rcsc_build_settings rcsc)
  add_test(NAME rundom_convex_hull COMMAND rundom_convex_hull)
  librcsc_set_unit_test_environment(rundom_convex_hull)
endif()
