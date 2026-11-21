
add_library(rcsc_color OBJECT
  rgb_color.cpp
  gradation_color_provider.cpp
  gray_scale_provider.cpp
  thermo_color_provider.cpp
  )

target_link_libraries(rcsc_color PRIVATE rcsc_build_settings)

install(FILES
  rgb_color.h
  gradation_color_provider.h
  gray_scale_provider.h
  thermo_color_provider.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rcsc/color
  )

# a program that prints the thermo colors in PostScript (built with the
# tests as in the Autotools build, but not run as a test)
if(BUILD_TESTING)
  add_executable(thermo_color_test thermo_color_test.cpp)
  target_link_libraries(thermo_color_test PRIVATE rcsc_build_settings rcsc)
endif()
