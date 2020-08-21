
add_library(librcsc_util SHARED
  game_mode.cpp
  soccer_math.cpp
  version.cpp
  )

target_include_directories(librcsc_util PUBLIC
  ${Boost_INCLUDE_DIRS}
  ${ZLIB_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
  )
