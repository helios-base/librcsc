
add_library(rcsc_gz OBJECT
  gzcompressor.cpp
  gzfstream.cpp
  gzfilterstream.cpp
  )

target_include_directories(rcsc_gz
  PUBLIC
  ${Boost_INCLUDE_DIRS}
  ${ZLIB_INCLUDE_DIRS}
  PRIVATE
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
  )

install(FILES
  gzcompressor.h
  gzfstream.h
  gzfilterstream.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rcsc/gz
  )
