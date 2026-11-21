# CheckInstalledTools.cmake
# -------------------------
#
# Check that the installed tools of librcsc run without LD_LIBRARY_PATH, and
# that they use librcsc in the same installation (i.e. the library path is
# recorded in the tools: RPATH/RUNPATH). Used by the "installed_tools" test,
# and by "cmake-helper.sh check-tools PREFIX" (also for librcsc installed by
# Autotools):
#
#   cmake -DPREFIX=<installation directory> -P test/CheckInstalledTools.cmake
#
# For each program in PREFIX/bin (the scripts are skipped):
#   - It is run without arguments (and with empty input), with LD_LIBRARY_PATH
#     unset. It must not fail to load the shared libraries. The exit code of
#     the usage message (e.g. 1) is accepted.
#   - If ldd is available, librcsc must be resolved to a file in PREFIX.
#     At least one program must be linked with librcsc.
#
# Note that a system directory (e.g. /usr/local/lib) is found by the dynamic
# linker without the recorded library path, after "ldconfig" is run.

if(NOT PREFIX)
  message(FATAL_ERROR "CheckInstalledTools.cmake: PREFIX is not set.")
endif()
if(NOT IS_DIRECTORY "${PREFIX}/bin")
  message(FATAL_ERROR "${PREFIX}/bin is not found.")
endif()

# the recorded library path must be used, not LD_LIBRARY_PATH
unset(ENV{LD_LIBRARY_PATH})
unset(ENV{DYLD_LIBRARY_PATH})

get_filename_component(prefix_real "${PREFIX}" REALPATH)
find_program(LDD_EXECUTABLE ldd)
set(input_option)
if(EXISTS /dev/null)
  set(input_option INPUT_FILE /dev/null)
endif()

file(GLOB programs LIST_DIRECTORIES false "${PREFIX}/bin/*")
list(SORT programs)

set(failed)
set(num_checked 0)
set(num_linked 0)
foreach(program IN LISTS programs)
  get_filename_component(name "${program}" NAME)
  # skip the scripts (e.g. librcsc-config)
  file(READ "${program}" head LIMIT 2 HEX)
  if(head STREQUAL "2321") # "#!"
    continue()
  endif()
  math(EXPR num_checked "${num_checked} + 1")

  execute_process(
    COMMAND "${program}"
    ${input_option}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE output
    TIMEOUT 30
    )
  if(NOT result MATCHES "^[0-9]+$" OR result EQUAL 127
      OR output MATCHES "error while loading shared libraries|Library not loaded|cannot open shared object")
    message(STATUS "${name}: failed to run (${result})\n${output}")
    list(APPEND failed "${name}")
    continue()
  endif()

  set(library "(not linked with librcsc)")
  if(LDD_EXECUTABLE)
    execute_process(
      COMMAND "${LDD_EXECUTABLE}" "${program}"
      OUTPUT_VARIABLE ldd_output
      ERROR_VARIABLE ldd_output
      )
    if(ldd_output MATCHES "librcsc[^ \t\n]* => ([^ \t\n]+)")
      math(EXPR num_linked "${num_linked} + 1")
      set(library "${CMAKE_MATCH_1}")
      get_filename_component(library_real "${library}" REALPATH)
      string(FIND "${library_real}" "${prefix_real}/" pos)
      if(NOT pos EQUAL 0)
        message(STATUS "${name}: librcsc is not found in ${PREFIX}: ${library}")
        list(APPEND failed "${name}")
        continue()
      endif()
    endif()
  endif()
  message(STATUS "${name}: OK (exit code ${result}, librcsc: ${library})")
endforeach()

if(num_checked EQUAL 0)
  message(FATAL_ERROR "No program is found in ${PREFIX}/bin")
endif()
if(LDD_EXECUTABLE AND num_linked EQUAL 0)
  message(FATAL_ERROR "No program in ${PREFIX}/bin is linked with librcsc")
endif()
if(failed)
  message(FATAL_ERROR "These programs failed: ${failed}")
endif()
message(STATUS "All the ${num_checked} programs in ${PREFIX}/bin run without LD_LIBRARY_PATH.")
