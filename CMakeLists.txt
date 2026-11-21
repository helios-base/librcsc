cmake_minimum_required(VERSION 3.16)

# NOTE: Update the version in configure.ac (AC_INIT) at the same time.
project(librcsc VERSION 2026 LANGUAGES CXX)

# NOTE: Update -version-info in rcsc/Makefile.am at the same time.
set(LIBRCSC_SOVERSION 20) # protocol version
set(LIBRCSC_BUILDVERSION ${LIBRCSC_SOVERSION}.0.0)

# compiler options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# install destination: ~/.local by default
# (~/.local/bin is in PATH on Ubuntu, and the other programs such as
# soccerwindow2 and helios-base search ~/.local for librcsc).
# A build directory configured before keeps its value in the cache.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "Install destination path" FORCE)
endif()
include(GNUInstallDirs)

# Embed the relative path from the installed programs to librcsc (RPATH), so
# that LD_LIBRARY_PATH is not needed to run them, as in the Autotools build.
# The relative path also works when the prefix is changed at the installation.
if(NOT WIN32)
  file(RELATIVE_PATH _librcsc_bin_to_lib
    "${CMAKE_INSTALL_FULL_BINDIR}" "${CMAKE_INSTALL_FULL_LIBDIR}")
  if(APPLE)
    set(CMAKE_INSTALL_RPATH "@loader_path/${_librcsc_bin_to_lib}")
  else()
    set(CMAKE_INSTALL_RPATH "$ORIGIN/${_librcsc_bin_to_lib}")
  endif()
endif()

# check header files
include(CheckIncludeFileCXX)
if(WIN32)
  check_include_file_cxx("windows.h" HAVE_WINDOWS_H)
endif()
check_include_file_cxx("arpa/inet.h" HAVE_ARPA_INET_H)
check_include_file_cxx("fcntl.h" HAVE_FCNTL_H)
check_include_file_cxx("netdb.h" HAVE_NETDB_H)
check_include_file_cxx("netinet/in.h" HAVE_NETINET_IN_H)
check_include_file_cxx("sys/socket.h" HAVE_SYS_SOCKET_H)
check_include_file_cxx("sys/time.h" HAVE_SYS_TIME_H)
check_include_file_cxx("unistd.h" HAVE_UNISTD_H)

# check funcs
include(CheckCXXSymbolExists)
check_cxx_symbol_exists(inet_addr arpa/inet.h HAVE_INET_ADDR)
check_cxx_symbol_exists(getaddrinfo netdb.h HAVE_GETADDRINFO)
check_cxx_symbol_exists(gethostbyname netdb.h HAVE_GETHOSTBYNAME)
check_cxx_symbol_exists(gettimeofday sys/time.h HAVE_GETTIMEOFDAY)
check_cxx_symbol_exists(select sys/select.h HAVE_SELECT)
check_cxx_symbol_exists(socket sys/socket.h HAVE_SOCKET)

# boost (header-only)
# Boost 1.70 or later provides BoostConfig.cmake, so the config mode is used
# regardless of the CMake version (the FindBoost module is not used).
find_package(Boost 1.70 CONFIG REQUIRED)

# zlib (optional, as in the Autotools build)
find_package(ZLIB)
if(ZLIB_FOUND)
  set(HAVE_LIBZ TRUE)
endif()

# generate config.h (used only to build librcsc, not installed)
configure_file(
  ${PROJECT_SOURCE_DIR}/cmake-config.h.in
  ${PROJECT_BINARY_DIR}/config.h
  )

# the settings to build librcsc and its programs (not exported):
# config.h, the include directories, and the warning options
add_library(rcsc_build_settings INTERFACE)
target_compile_definitions(rcsc_build_settings INTERFACE HAVE_CONFIG_H)
target_include_directories(rcsc_build_settings INTERFACE
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
  )
target_compile_options(rcsc_build_settings INTERFACE
  "$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-W;-Wall>"
  )

# check the settings
message(STATUS "Build settings:")
message(STATUS "  BUILD_TYPE=${CMAKE_BUILD_TYPE}")
message(STATUS "  INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")

# sub directories
add_subdirectory(rcsc)
add_subdirectory(src)

# additional installation files
set(PACKAGE ${PROJECT_NAME})
set(VERSION ${librcsc_VERSION})
set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix ${CMAKE_INSTALL_PREFIX})
set(libdir ${CMAKE_INSTALL_FULL_LIBDIR})
set(includedir ${CMAKE_INSTALL_FULL_INCLUDEDIR})
set(builddir ${CMAKE_BINARY_DIR})
set(top_srcdir ${PROJECT_SOURCE_DIR})
set(HAVE_DOT "")

configure_file(Doxyfile.in Doxyfile @ONLY)
configure_file(librcsc-config.in librcsc-config @ONLY)
configure_file(librcscenv.in librcscenv @ONLY)
configure_file(librcsc.pc.in librcsc.pc @ONLY)

install(PROGRAMS
  ${CMAKE_CURRENT_BINARY_DIR}/librcsc-config
  ${CMAKE_CURRENT_BINARY_DIR}/librcscenv
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  )

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/librcsc.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
  )

# CMake package configuration files (lib/cmake/librcsc), with which the other
# CMake projects can use librcsc by
#   find_package(librcsc CONFIG REQUIRED)
#   target_link_libraries(<target> PRIVATE librcsc::rcsc)
# They are installed only by the CMake build, not by the Autotools build.
include(CMakePackageConfigHelpers)
set(LIBRCSC_CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/librcsc)
install(EXPORT librcscTargets
  NAMESPACE librcsc::
  DESTINATION ${LIBRCSC_CONFIG_INSTALL_DIR}
  )
configure_package_config_file(
  ${PROJECT_SOURCE_DIR}/cmake/librcscConfig.cmake.in
  ${PROJECT_BINARY_DIR}/librcscConfig.cmake
  INSTALL_DESTINATION ${LIBRCSC_CONFIG_INSTALL_DIR}
  )
# The version is a year (e.g. 2026). A newer librcsc satisfies the request of
# an older version, e.g. find_package(librcsc 2024) finds librcsc 2026.
write_basic_package_version_file(
  ${PROJECT_BINARY_DIR}/librcscConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
  )
install(FILES
  ${PROJECT_BINARY_DIR}/librcscConfig.cmake
  ${PROJECT_BINARY_DIR}/librcscConfigVersion.cmake
  DESTINATION ${LIBRCSC_CONFIG_INSTALL_DIR}
  )

# uninstall: cmake --build build --target uninstall
# (removes the files listed in install_manifest.txt of the build directory)
add_custom_target(uninstall
  COMMAND ${CMAKE_COMMAND}
          -DMANIFEST=${PROJECT_BINARY_DIR}/install_manifest.txt
          "-DPACKAGE_DIRS=${CMAKE_INSTALL_INCLUDEDIR}/rcsc$<SEMICOLON>${LIBRCSC_CONFIG_INSTALL_DIR}"
          -P ${PROJECT_SOURCE_DIR}/cmake/Uninstall.cmake
  COMMENT "Uninstalling ${PROJECT_NAME}"
  VERBATIM
  )
