
add_library(rcsc_clang OBJECT
  clang_action.cpp
  clang_condition.cpp
  clang_directive.cpp
  clang_info_message.cpp
  clang_parser.cpp
  clang_token.cpp
  clang_unum.cpp
  )

target_include_directories(rcsc_clang
  PUBLIC
  ${Boost_INCLUDE_DIRS}
  PRIVATE
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
  )

install(FILES
  clang_action.h
  clang_condition.h
  clang_directive.h
  clang_info_message.h
  clang_message.h
  clang_parser.h
  clang_token.h
  clang_unum.h
  clang.h
  types.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rcsc/clang
  )
