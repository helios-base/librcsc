
# rclmscheduler and rclmtableprinter do not use librcsc.
add_executable(rclmscheduler
  scheduler.cpp
  )

add_executable(rclmtableprinter
  tableprinter.cpp
  )

add_executable(rcg2csv
  rcg2csv.cpp
  )
target_link_libraries(rcg2csv PRIVATE
  rcsc
  )

add_executable(rcg2txt
  rcg2txt.cpp
  )
target_link_libraries(rcg2txt PRIVATE
  rcsc
  )

add_executable(rcgrenameteam
  rcgrenameteam.cpp
  )
target_link_libraries(rcgrenameteam PRIVATE
  rcsc
  )

add_executable(rcgresultprinter
  resultprinter.cpp
  )
target_link_libraries(rcgresultprinter PRIVATE
  rcsc
  )

add_executable(rcgreverse
  rcgreverse.cpp
  )
target_link_libraries(rcgreverse PRIVATE
  rcsc
  )

add_executable(rcgvalidator
  rcgvalidator.cpp
  )
target_link_libraries(rcgvalidator PRIVATE
  rcsc
  )

add_executable(rcgverconv
  rcgverconv.cpp
  )
target_link_libraries(rcgverconv PRIVATE
  rcsc
  )

add_executable(rcgversion
  rcgversion.cpp
  )
target_link_libraries(rcgversion PRIVATE
  rcsc
  )

# a tool for the development (not installed)
add_executable(object_table_printer
  object_table_printer.cpp
  )
target_link_libraries(object_table_printer PRIVATE
  rcsc
  )

include_directories(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
  )

install(TARGETS
  rclmscheduler
  rclmtableprinter
  rcg2csv
  rcg2txt
  rcgrenameteam
  rcgresultprinter
  rcgreverse
  rcgvalidator
  rcgverconv
  rcgversion
  RUNTIME
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
