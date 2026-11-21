# BuildTestProject.cmake
# ----------------------
#
# Configure and build a test project, and run a program built by it (used by
# the "installed_headers" and "find_package" tests). The project is built in
# parallel, which "ctest --build-and-test" does not do.
#
#   cmake -DSOURCE_DIR=... -DBINARY_DIR=... [-DGENERATOR=...] [-DCONFIG=...]
#         [-DJOBS=...] [-DRUN=program] -P BuildTestProject.cmake
#         -- [options for configuring the project...]
#
# Input variables (-D<var>=<value>):
#   SOURCE_DIR  the source directory of the project
#   BINARY_DIR  the build directory of the project
#   GENERATOR   the CMake generator (optional)
#   CONFIG      the build configuration (optional)
#   JOBS        the number of parallel build jobs (default: the number of CPUs)
#   RUN         the name of a program to run after the build (optional)
# The arguments after "--" are passed to the configuration of the project.

foreach(var SOURCE_DIR BINARY_DIR)
  if(NOT ${var})
    message(FATAL_ERROR "BuildTestProject.cmake: ${var} is not set.")
  endif()
endforeach()

# the options after "--"
set(configure_options)
set(after_separator FALSE)
math(EXPR last_index "${CMAKE_ARGC} - 1")
foreach(i RANGE ${last_index})
  if(after_separator)
    list(APPEND configure_options "${CMAKE_ARGV${i}}")
  elseif("${CMAKE_ARGV${i}}" STREQUAL "--")
    set(after_separator TRUE)
  endif()
endforeach()

set(generator_option)
if(GENERATOR)
  set(generator_option -G "${GENERATOR}")
endif()
set(config_option)
if(CONFIG)
  set(config_option --config "${CONFIG}")
endif()
if(NOT JOBS)
  cmake_host_system_information(RESULT JOBS QUERY NUMBER_OF_LOGICAL_CORES)
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" -S "${SOURCE_DIR}" -B "${BINARY_DIR}"
          ${generator_option} ${configure_options}
  RESULT_VARIABLE result
  )
if(NOT result EQUAL 0)
  message(FATAL_ERROR "Failed to configure ${SOURCE_DIR}")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" --build "${BINARY_DIR}" --parallel ${JOBS} ${config_option}
  RESULT_VARIABLE result
  )
if(NOT result EQUAL 0)
  message(FATAL_ERROR "Failed to build ${SOURCE_DIR}")
endif()

if(RUN)
  # single-config generators: BINARY_DIR/RUN, multi-config: BINARY_DIR/CONFIG/RUN
  set(program "${BINARY_DIR}/${RUN}")
  if(NOT EXISTS "${program}" AND CONFIG)
    set(program "${BINARY_DIR}/${CONFIG}/${RUN}")
  endif()
  execute_process(COMMAND "${program}" RESULT_VARIABLE result)
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "${program} failed (${result})")
  endif()
endif()
