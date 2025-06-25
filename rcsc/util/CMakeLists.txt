add_library(rcsc_util OBJECT
  game_mode.cpp
  soccer_math.cpp
  version.cpp
  performance_monitor.cpp
  memory_pool.cpp
  )

target_include_directories(rcsc_util
  PUBLIC
  ${Boost_INCLUDE_DIRS}
  PRIVATE
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
  )

# Enable C++17 features for better performance
target_compile_features(rcsc_util PRIVATE cxx_std_17)
