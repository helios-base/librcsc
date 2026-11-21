# Uninstall.cmake
# ---------------
#
# Remove the files installed by "cmake --install". This script is run by the
# "uninstall" target:
#
#   cmake --build build --target uninstall
#
# The files listed in install_manifest.txt (created in the build directory by
# the install) are removed. The package directories (e.g. include/rcsc and
# lib/cmake/librcsc) and their subdirectories are also removed if they become
# empty. The other directories (e.g. bin and lib) are not removed.
#
# Input variables (-D<var>=<value>):
#   MANIFEST      the path of install_manifest.txt
#   PACKAGE_DIRS  the package directories relative to the installation prefix
#                 (a list, e.g. include/rcsc;lib/cmake/librcsc)

foreach(var MANIFEST PACKAGE_DIRS)
  if(NOT ${var})
    message(FATAL_ERROR "Uninstall.cmake: ${var} is not set.")
  endif()
endforeach()

if(NOT EXISTS "${MANIFEST}")
  message(FATAL_ERROR "${MANIFEST} is not found. "
    "The uninstall target removes the files installed from this build directory.")
endif()

file(STRINGS "${MANIFEST}" installed_files)

# the directories that may become empty: the directory of each removed file
# and its parents up to the package directory
set(candidate_dirs)
foreach(file IN LISTS installed_files)
  set(path "$ENV{DESTDIR}${file}")
  if(IS_SYMLINK "${path}" OR EXISTS "${path}")
    message(STATUS "Uninstalling: ${path}")
    file(REMOVE "${path}")
    if(IS_SYMLINK "${path}" OR EXISTS "${path}")
      message(FATAL_ERROR "Failed to remove ${path}")
    endif()
  else()
    message(STATUS "Not found (skipped): ${path}")
  endif()
  foreach(package_dir IN LISTS PACKAGE_DIRS)
    # the package directory in the installation (e.g. /PREFIX/include/rcsc)
    string(FIND "${path}" "/${package_dir}/" pos REVERSE)
    if(pos EQUAL -1)
      continue()
    endif()
    string(LENGTH "/${package_dir}" len)
    math(EXPR end "${pos} + ${len}")
    string(SUBSTRING "${path}" 0 ${end} root)
    get_filename_component(dir "${path}" DIRECTORY)
    # walk up to the package directory (stop also when the path gets shorter
    # than it, which happens only if the path is not normalized)
    string(LENGTH "${root}" root_len)
    string(LENGTH "${dir}" dir_len)
    while(dir_len GREATER_EQUAL root_len)
      list(APPEND candidate_dirs "${dir}")
      if(dir STREQUAL root)
        break()
      endif()
      get_filename_component(dir "${dir}" DIRECTORY)
      string(LENGTH "${dir}" dir_len)
    endwhile()
  endforeach()
endforeach()

# Remove the empty directories, the deepest first.
if(candidate_dirs)
  list(REMOVE_DUPLICATES candidate_dirs)
  list(SORT candidate_dirs ORDER DESCENDING)
endif()
foreach(dir IN LISTS candidate_dirs)
  if(NOT IS_DIRECTORY "${dir}")
    continue()
  endif()
  file(GLOB entries LIST_DIRECTORIES true "${dir}/*" "${dir}/.*")
  list(FILTER entries EXCLUDE REGEX "/\\.\\.?$")
  if(NOT entries)
    message(STATUS "Removing the empty directory: ${dir}")
    file(REMOVE_RECURSE "${dir}")
  endif()
endforeach()
