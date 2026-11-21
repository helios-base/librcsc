
add_library(rcsc_util OBJECT
  game_mode.cpp
  soccer_math.cpp
  version.cpp
  )

target_link_libraries(rcsc_util PRIVATE rcsc_build_settings)
