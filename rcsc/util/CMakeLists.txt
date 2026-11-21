
add_library(rcsc_util OBJECT
  game_mode.cpp
  soccer_math.cpp
  version.cpp
  )

target_include_directories(rcsc_util
  PRIVATE
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
  )
