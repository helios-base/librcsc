
add_library(rcsc_ann OBJECT
  ngnet.cpp
  rbf.cpp
  sirm.cpp
  sirms_model.cpp
  )

target_include_directories(rcsc_ann
  PUBLIC
  ${Boost_INCLUDE_DIRS}
  PRIVATE
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
  )

install(FILES
  bpn1.h
  ngnet.h
  rbf.h
  sirm.h
  sirms_model.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rcsc/ann
  )
