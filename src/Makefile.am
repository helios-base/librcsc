
bin_PROGRAMS = \
	rclmscheduler \
	rclmtableprinter \
	rcg2csv \
	rcg2txt \
	rcgrenameteam \
	rcgresultprinter \
	rcgreverse \
	rcgverconv \
	rcgversion

noinst_PROGRAMS = \
	object_table_printer

rclmscheduler_SOURCES = \
	scheduler.cpp
rclmscheduler_CXXFLAGS = -Wall -W
rclmscheduler_LDFLAGS =
rclmscheduler_LDADD =

rclmtableprinter_SOURCES = \
	tableprinter.cpp
rclmtableprinter_CXXFLAGS = -Wall -W
rclmtableprinter_LDFLAGS =
rclmtableprinter_LDADD =


rcgresultprinter_SOURCES = \
	resultprinter.cpp
rcgresultprinter_CXXFLAGS = -Wall -W
rcgresultprinter_LDFLAGS = \
	-L$(top_builddir)/rcsc
rcgresultprinter_LDADD = -lrcsc  $(BOOST_SYSTEM_LIB)

rcg2csv_SOURCES = \
	rcg2csv.cpp
rcg2csv_CXXFLAGS = -Wall -W
rcg2csv_LDFLAGS = \
	-L$(top_builddir)/rcsc
rcg2csv_LDADD = -lrcsc $(BOOST_SYSTEM_LIB)

rcg2txt_SOURCES = \
	rcg2txt.cpp
rcg2txt_CXXFLAGS = -Wall -W
rcg2txt_LDFLAGS = \
	-L$(top_builddir)/rcsc
rcg2txt_LDADD = -lrcsc $(BOOST_SYSTEM_LIB)

rcgrenameteam_SOURCES = \
	rcgrenameteam.cpp
rcgrenameteam_CXXFLAGS = -Wall -W
rcgrenameteam_LDFLAGS = \
	-L$(top_builddir)/rcsc
rcgrenameteam_LDADD = -lrcsc $(BOOST_SYSTEM_LIB)

rcgreverse_SOURCES = \
	rcgreverse.cpp
rcgreverse_CXXFLAGS = -Wall -W
rcgreverse_LDFLAGS = \
	-L$(top_builddir)/rcsc
rcgreverse_LDADD = -lrcsc $(BOOST_SYSTEM_LIB)

rcgverconv_SOURCES = \
	rcgverconv.cpp
rcgversion_CXXFLAGS = -Wall -W
rcgverconv_LDFLAGS = \
	-L$(top_builddir)/rcsc
rcgverconv_LDADD = -lrcsc $(BOOST_SYSTEM_LIB)

rcgversion_SOURCES = \
	rcgversion.cpp
rcgversion_LDFLAGS = \
	-L$(top_builddir)/rcsc
rcgversion_LDADD = -lrcsc $(BOOST_SYSTEM_LIB)

object_table_printer_SOURCES = \
	object_table_printer.cpp
object_table_printer_LDFLAGS = \
	-L$(top_builddir)/rcsc
object_table_printer_LDADD = -lrcsc $(BOOST_SYSTEM_LIB)

AM_CPPFLAGS = -I$(top_srcdir)
AM_CXXFLAGS = -Wall -W
AM_CFLAGS = -Wall -W
AM_LDFLAGS =

CLEANFILES = *~
