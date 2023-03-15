cmake_minimum_required(VERSION 3.5)

project(librcsc VERSION 2023)

set(LIBRCSC_SOVERSION 18) # protocol version
set(LIBRCSC_BUILDVERSION ${LIBRCSC_SOVERSION}.0.0)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

# compliler options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()
set(CMAKE_CXX_FLAGS "-W -Wall")

# install destination
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/local" CACHE PATH "Install destination path" FORCE)
endif()

include(GNUInstallDirs)

# check header files
include(CheckIncludeFileCXX)
if(WIN32)
  check_include_file_cxx("windows.h" HAVE_WINDOWS_H)
endif()
check_include_file_cxx("arpa/inet.h" HAVE_ARPA_INET_H)
check_include_file_cxx("fcntl.h" HAVE_FCNTL_H)
check_include_file_cxx("netinet/in.h" HAVE_NETINET_IN_H)
check_include_file_cxx("netdb.h" HAVE_NETDB_H)
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

# boost
find_package(Boost 1.41.0 COMPONENTS system REQUIRED)
if(NOT Boost_FOUND)
  message(FATAL_ERROR "Boost not found!")
endif()

# zlib
find_package(ZLIB)
if(ZLIB_FOUND)
  #add_definitions(-DHAVE_LIBZ)
  set(HAVE_LIBZ TRUE)
endif()

# generate config.h
add_definitions(-DHAVE_CONFIG_H)
configure_file(
  ${PROJECT_SOURCE_DIR}/cmake-config.h.in
  ${PROJECT_BINARY_DIR}/config.h
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
  DESTINATION bin
  )

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/librcsc.pc
  DESTINATION lib/pkgconfig
  )
