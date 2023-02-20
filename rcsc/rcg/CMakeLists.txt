
add_library(rcsc_rcg OBJECT
	handler.cpp
	parser.cpp
	parser_v1.cpp
	parser_v2.cpp
	parser_v3.cpp
	parser_v4.cpp
	serializer.cpp
	serializer_v1.cpp
	serializer_v2.cpp
	serializer_v3.cpp
	serializer_v4.cpp
	serializer_v5.cpp
	serializer_v6.cpp
	util.cpp
  )

target_include_directories(rcsc_rcg
  PUBLIC
  ${Boost_INCLUDE_DIRS}
  PRIVATE
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
  )

install(FILES
  handler.h
  parser.h
  parser_v1.h
  parser_v2.h
  parser_v3.h
  parser_v4.h
  serializer.h
  serializer_v1.h
  serializer_v2.h
  serializer_v3.h
  serializer_v4.h
  serializer_v5.h
  serializer_v6.h
  types.h
  util.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rcsc/rcg
  )
