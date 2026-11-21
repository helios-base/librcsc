# Check that a CMake project can use the installed librcsc by
# find_package(librcsc CONFIG) and the imported target librcsc::rcsc.
#
# This project is built and run by the "find_package" test of librcsc, with
# CMAKE_PREFIX_PATH set to the installation for the tests.
#
# Optional variables:
#   EXPECTED_LIBRCSC_DIR  the directory where librcscConfig.cmake must be found
#                         (to check that no other installation is used)
#   EXPECTED_VERSION      the version that the library must report

cmake_minimum_required(VERSION 3.16)
project(librcsc_find_package_test LANGUAGES CXX)

find_package(librcsc CONFIG REQUIRED)
message(STATUS "Found librcsc ${librcsc_VERSION}: ${librcsc_DIR}")

if(EXPECTED_LIBRCSC_DIR)
  get_filename_component(found_dir "${librcsc_DIR}" REALPATH)
  get_filename_component(expected_dir "${EXPECTED_LIBRCSC_DIR}" REALPATH)
  if(NOT found_dir STREQUAL expected_dir)
    message(FATAL_ERROR "librcsc is found in ${found_dir}, not in ${expected_dir}")
  endif()
endif()

add_executable(find_package_test main.cpp)
target_link_libraries(find_package_test PRIVATE librcsc::rcsc)
if(EXPECTED_VERSION)
  target_compile_definitions(find_package_test PRIVATE
    LIBRCSC_EXPECTED_VERSION="${EXPECTED_VERSION}"
    )
endif()
