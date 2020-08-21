cmake_minimum_required(VERSION 3.5)

project(librcsc VERSION 2020.08)

set(LIBRCSC_SOVERSION 16) # protocol version
set(LIBRCSC_BUILDVERSION ${LIBRCSC_SOVERSION}.0.0)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

# c++ version
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# install destination
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  #  set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "..." FORCE)
  set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/test" CACHE PATH "..." FORCE)
endif()

# check header files
include(CheckIncludeFile)
if(WIN32)
  check_include_file("windows.h" HAVE_WINDOWS_H)
endif()
check_include_file("arpa/inet.h" HAVE_ARPA_INET_H)
check_include_file("fcntl.h" HAVE_FCNTL_H)
check_include_file("netinet/in.h" HAVE_NETINET_IN_H)
check_include_file("netdb.h" HAVE_NETDB_H)
check_include_file("sys/socket.h" HAVE_SYS_SOCKET_H)
check_include_file("sys/time.h" HAVE_SYS_TIME_H)
check_include_file("unistd.h" HAVE_UNISTD_H)

# libraries
find_package(Boost 1.36.0 COMPONENTS system REQUIRED)
find_package(ZLIB)
if(ZLIB_FOUND)
  #add_definitions(-DHAVE_LIBZ)
  set(HAVE_LIBZ TRUE)
endif()

# generate config.h
add_definitions(-DHAVE_CONFIG_H)
configure_file(
  ${PROJECT_SOURCE_DIR}/config.h.cmake
  ${PROJECT_BINARY_DIR}/config.h
  )

# sub directories
add_subdirectory(rcsc)
