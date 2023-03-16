
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # need for linking OBJCET libraries to a shared library

add_subdirectory(util)
add_subdirectory(geom)
add_subdirectory(gz)
add_subdirectory(param)
add_subdirectory(rcg)
add_subdirectory(net)
add_subdirectory(time)
add_subdirectory(ann)
add_subdirectory(clang)
add_subdirectory(formation)
add_subdirectory(color)
add_subdirectory(common)
add_subdirectory(monitor)
add_subdirectory(player)
add_subdirectory(coach)
add_subdirectory(trainer)

add_library(rcsc SHARED
  $<TARGET_OBJECTS:rcsc_util>
  $<TARGET_OBJECTS:rcsc_geom_triangle>
  $<TARGET_OBJECTS:rcsc_geom>
  $<TARGET_OBJECTS:rcsc_gz>
  $<TARGET_OBJECTS:rcsc_param>
  $<TARGET_OBJECTS:rcsc_rcg>
  $<TARGET_OBJECTS:rcsc_net>
  $<TARGET_OBJECTS:rcsc_time>
  $<TARGET_OBJECTS:rcsc_ann>
  $<TARGET_OBJECTS:rcsc_clang>
  $<TARGET_OBJECTS:rcsc_formation>
  $<TARGET_OBJECTS:rcsc_color>
  $<TARGET_OBJECTS:rcsc_common>
  $<TARGET_OBJECTS:rcsc_monitor>
  $<TARGET_OBJECTS:rcsc_player>
  $<TARGET_OBJECTS:rcsc_coach>
  $<TARGET_OBJECTS:rcsc_trainer>
  )

target_include_directories(rcsc
  PUBLIC
  ${Boost_INCLUDE_DIRS}
  ${ZLIB_INCLUDE_DIRS}
  PRIVATE
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
#  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
#  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
#  $<INSTALL_INTERFACE:include>
  )

set_target_properties(rcsc PROPERTIES
  VERSION ${LIBRCSC_BUILDVERSION}
  SOVERSION ${LIBRCSC_SOVERSION}
  )

# install the library file
install(TARGETS rcsc
#  EXPORT librcsc-export # for install(EXPORT)
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT Libraries
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
#  INCLUDES DESTINATION include # for install(EXPORT)
  )

# install header files
install(FILES
  types.h
  geom.h
  gz.h
  rcg.h
  factory.h
  game_mode.h
  game_time.h
  math_util.h
  random.h
  soccer_math.h
  timer.h
  version.h
  #TYPE INCLUDE # available on cmake-3.14 or later
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rcsc
  )

# generated .cmake file
#install(EXPORT librcsc-export
#  FILE librcsc-config.cmake
#  DESTINATION ${CMAKE_INSTALL_PREFIX}/share/cmake/librcsc/
#  EXPORT_LINK_INTERFACE_LIBRARIES
#)
