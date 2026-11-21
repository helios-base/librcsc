# Tests of the installation of librcsc.
#
# The "install" test installs librcsc into a directory in the build directory
# (test-install), and the other tests use it:
#   installed_headers  each installed header file can be compiled alone
#                      (e.g. no header file is missing in the installation)
#   find_package       a CMake project can use the installed librcsc by
#                      find_package(librcsc CONFIG) and librcsc::rcsc
#   installed_tools    the installed tools run without LD_LIBRARY_PATH and use
#                      librcsc in the same installation (the recorded RPATH)
#
# The projects used by the tests (installed_headers/ and find_package/) are
# configured and built in this directory of the build tree by
# BuildTestProject.cmake.

set(LIBRCSC_TEST_PREFIX ${PROJECT_BINARY_DIR}/test-install)

add_test(NAME install
  COMMAND ${CMAKE_COMMAND}
          -DBUILD_DIR=${PROJECT_BINARY_DIR}
          -DPREFIX=${LIBRCSC_TEST_PREFIX}
          -DCONFIG=$<CONFIG>
          -P ${CMAKE_CURRENT_SOURCE_DIR}/InstallForTest.cmake
  )
set_tests_properties(install PROPERTIES FIXTURES_SETUP librcsc_installed)

# configure the test projects with the same generator and compiler
set(_librcsc_test_project_options
  -DGENERATOR=${CMAKE_GENERATOR}
  -DCONFIG=$<CONFIG>
  )
set(_librcsc_test_configure_options
  -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  -DCMAKE_BUILD_TYPE=$<CONFIG>
  )

add_test(NAME installed_headers
  COMMAND ${CMAKE_COMMAND}
          -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/installed_headers
          -DBINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}/installed_headers
          ${_librcsc_test_project_options}
          -P ${CMAKE_CURRENT_SOURCE_DIR}/BuildTestProject.cmake
          --
          ${_librcsc_test_configure_options}
          -DLIBRCSC_PREFIX=${LIBRCSC_TEST_PREFIX}
  )
set_tests_properties(installed_headers PROPERTIES FIXTURES_REQUIRED librcsc_installed)

add_test(NAME find_package
  COMMAND ${CMAKE_COMMAND}
          -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/find_package
          -DBINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}/find_package
          ${_librcsc_test_project_options}
          -DRUN=find_package_test
          -P ${CMAKE_CURRENT_SOURCE_DIR}/BuildTestProject.cmake
          --
          ${_librcsc_test_configure_options}
          -DCMAKE_PREFIX_PATH=${LIBRCSC_TEST_PREFIX}
          -DEXPECTED_LIBRCSC_DIR=${LIBRCSC_TEST_PREFIX}/${LIBRCSC_CONFIG_INSTALL_DIR}
          -DEXPECTED_VERSION=${PROJECT_VERSION}
  )
set_tests_properties(find_package PROPERTIES FIXTURES_REQUIRED librcsc_installed)

add_test(NAME installed_tools
  COMMAND ${CMAKE_COMMAND}
          -DPREFIX=${LIBRCSC_TEST_PREFIX}
          -P ${CMAKE_CURRENT_SOURCE_DIR}/CheckInstalledTools.cmake
  )
set_tests_properties(installed_tools PROPERTIES FIXTURES_REQUIRED librcsc_installed)
