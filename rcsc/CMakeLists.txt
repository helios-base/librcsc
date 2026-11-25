
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # need for linking OBJECT libraries to a shared library

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

# the include directory of the public headers (the top source directory in
# the build tree, PREFIX/include in the installation), and C++17
target_include_directories(rcsc PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )
target_compile_features(rcsc PUBLIC cxx_std_17)

# zlib is used only inside librcsc (the public headers do not include zlib.h)
if(ZLIB_FOUND)
  target_link_libraries(rcsc PRIVATE ZLIB::ZLIB)
endif()

# the rcg parser uses std::thread (only inside librcsc)
target_link_libraries(rcsc PRIVATE Threads::Threads)

set_target_properties(rcsc PROPERTIES
  VERSION ${LIBRCSC_BUILDVERSION}
  SOVERSION ${LIBRCSC_SOVERSION}
  )

# the same name as the imported target of librcscConfig.cmake
add_library(librcsc::rcsc ALIAS rcsc)

# install the library file, and export the target to librcscTargets.cmake
# (installed by install(EXPORT) in the top CMakeLists.txt)
install(TARGETS rcsc
  EXPORT librcscTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
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
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rcsc
  )
