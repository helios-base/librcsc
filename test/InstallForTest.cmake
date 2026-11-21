# InstallForTest.cmake
# --------------------
#
# Install librcsc into a directory for the tests (run by the "install" test).
# The directory is removed before the installation, so that no file of an
# older installation remains.
#
# "cmake --install" writes install_manifest.txt in the build directory, which
# the uninstall target uses to remove the files installed by the user. It is
# saved before the installation for the tests and restored after it.
#
# Input variables (-D<var>=<value>):
#   BUILD_DIR  the build directory of librcsc
#   PREFIX     the installation directory for the tests
#   CONFIG     the build configuration (e.g. Release)

foreach(var BUILD_DIR PREFIX)
  if(NOT ${var})
    message(FATAL_ERROR "InstallForTest.cmake: ${var} is not set.")
  endif()
endforeach()

set(manifest "${BUILD_DIR}/install_manifest.txt")
set(saved_manifest "${BUILD_DIR}/install_manifest.txt.saved-by-test")

file(REMOVE_RECURSE "${PREFIX}")

if(EXISTS "${manifest}")
  file(RENAME "${manifest}" "${saved_manifest}")
endif()

set(config_option)
if(CONFIG)
  set(config_option --config "${CONFIG}")
endif()
execute_process(
  COMMAND "${CMAKE_COMMAND}" --install "${BUILD_DIR}" --prefix "${PREFIX}" ${config_option}
  RESULT_VARIABLE result
  )

file(REMOVE "${manifest}")
if(EXISTS "${saved_manifest}")
  file(RENAME "${saved_manifest}" "${manifest}")
endif()

if(NOT result EQUAL 0)
  message(FATAL_ERROR "Failed to install librcsc into ${PREFIX}")
endif()
message(STATUS "Installed librcsc into ${PREFIX}")
