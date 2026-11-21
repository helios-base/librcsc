
add_library(rcsc_ann OBJECT
  ngnet.cpp
  rbf.cpp
  sirm.cpp
  sirms_model.cpp
  )

target_link_libraries(rcsc_ann PRIVATE rcsc_build_settings)

install(FILES
  bpn1.h
  ngnet.h
  rbf.h
  sirm.h
  sirms_model.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rcsc/ann
  )
