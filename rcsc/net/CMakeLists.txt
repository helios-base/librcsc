
add_library(rcsc_net OBJECT
  abstract_socket.cpp
  host_address.cpp
  udp_socket.cpp
  tcp_socket.cpp
  )

target_link_libraries(rcsc_net PRIVATE rcsc_build_settings)

install(FILES
  abstract_socket.h
  host_address.h
  udp_socket.h
  tcp_socket.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rcsc/net
  )
