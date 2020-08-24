
add_library(rcsc_net OBJECT
  abstract_socket.cpp
  host_address.cpp
  udp_socket.cpp
  tcp_socket.cpp
  )

target_include_directories(rcsc_net
  PUBLIC
  ${Boost_INCLUDE_DIRS}
  PRIVATE
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
  )

install(FILES
  abstract_socket.h
  host_address.h
  udp_socket.h
  tcp_socket.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rcsc/net
  )
