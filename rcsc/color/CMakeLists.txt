
add_library(rcsc_color OBJECT
  rgb_color.cpp
  gradation_color_provider.cpp
  gray_scale_provider.cpp
  thermo_color_provider.cpp
  )

target_include_directories(rcsc_color
  PUBLIC
  ${Boost_INCLUDE_DIRS}
  PRIVATE
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
  )

install(FILES
  rgb_color.h
  gradation_color_provider.h
  gray_scale_provider.h
  thermo_color_provider.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rcsc/color
  )
