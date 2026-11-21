
add_library(rcsc_gz OBJECT
  gzcompressor.cpp
  gzfstream.cpp
  gzfilterstream.cpp
  )

target_link_libraries(rcsc_gz PRIVATE rcsc_build_settings)
if(ZLIB_FOUND)
  target_link_libraries(rcsc_gz PRIVATE ZLIB::ZLIB)
endif()

install(FILES
  gzcompressor.h
  gzfstream.h
  gzfilterstream.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rcsc/gz
  )
