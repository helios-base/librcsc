# Check that each installed header file of librcsc can be compiled alone
# (e.g. no header file that it includes is missing in the installation).
#
# This project is built by the "installed_headers" test of librcsc. It can
# also check librcsc installed by Autotools, e.g. in ~/.local:
#
#   cmake -S test/installed_headers -B /tmp/check-headers -DLIBRCSC_PREFIX=$HOME/.local
#   cmake --build /tmp/check-headers -- -k
#
# A source file that includes only one header file is generated for each
# installed header file (PREFIX/include/rcsc/**/*.h), and all of them are
# compiled with C++17 and only PREFIX/include as the include directory of
# librcsc.

cmake_minimum_required(VERSION 3.16)
project(librcsc_installed_headers LANGUAGES CXX)

set(LIBRCSC_PREFIX "" CACHE PATH "The installation directory of librcsc")
if(NOT LIBRCSC_PREFIX)
  message(FATAL_ERROR "Give the installation directory of librcsc by -DLIBRCSC_PREFIX=DIR")
endif()

set(include_dir "${LIBRCSC_PREFIX}/include")
file(GLOB_RECURSE headers RELATIVE "${include_dir}" "${include_dir}/rcsc/*.h")
if(NOT headers)
  message(FATAL_ERROR "No header file of librcsc is found in ${include_dir}/rcsc")
endif()
list(SORT headers)
list(LENGTH headers num_headers)
message(STATUS "Checking ${num_headers} header files in ${include_dir}/rcsc")

set(sources)
foreach(header IN LISTS headers)
  string(MAKE_C_IDENTIFIER "${header}" name)
  set(source "${CMAKE_CURRENT_BINARY_DIR}/src/${name}.cpp")
  set(content "#include <${header}>\n")
  # rewrite the file only when the content changes (to avoid rebuilding)
  set(old_content)
  if(EXISTS "${source}")
    file(READ "${source}" old_content)
  endif()
  if(NOT old_content STREQUAL content)
    file(WRITE "${source}" "${content}")
  endif()
  list(APPEND sources "${source}")
endforeach()

add_library(check_headers OBJECT ${sources})
target_include_directories(check_headers PRIVATE "${include_dir}")
set_target_properties(check_headers PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
  )
target_compile_options(check_headers PRIVATE
  "$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-W;-Wall>"
  )
