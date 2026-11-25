
bin_PROGRAMS = \
	rclmscheduler \
	rclmtableprinter \
	rcg2csv \
	rcg2txt \
	rcgrenameteam \
	rcgresultprinter \
	rcgreverse \
	rcgvalidator \
	rcgverconv \
	rcgversion

noinst_PROGRAMS = \
	object_table_printer \
	rcg_parser_bench

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
rcgresultprinter_LDADD = -lrcsc

rcg2csv_SOURCES = \
	rcg2csv.cpp
rcg2csv_CXXFLAGS = -Wall -W
rcg2csv_LDFLAGS = \
	-L$(top_builddir)/rcsc
rcg2csv_LDADD = -lrcsc

rcg2txt_SOURCES = \
	rcg2txt.cpp
rcg2txt_CXXFLAGS = -Wall -W
rcg2txt_LDFLAGS = \
	-L$(top_builddir)/rcsc
rcg2txt_LDADD = -lrcsc

rcgrenameteam_SOURCES = \
	rcgrenameteam.cpp
rcgrenameteam_CXXFLAGS = -Wall -W
rcgrenameteam_LDFLAGS = \
	-L$(top_builddir)/rcsc
rcgrenameteam_LDADD = -lrcsc

rcgreverse_SOURCES = \
	rcgreverse.cpp
rcgreverse_CXXFLAGS = -Wall -W
rcgreverse_LDFLAGS = \
	-L$(top_builddir)/rcsc
rcgreverse_LDADD = -lrcsc

rcgvalidator_SOURCES = \
	rcgvalidator.cpp
rcgvalidator_CXXFLAGS = -Wall -W
rcgvalidator_LDFLAGS = \
	-L$(top_builddir)/rcsc
rcgvalidator_LDADD = -lrcsc

rcgverconv_SOURCES = \
	rcgverconv.cpp
rcgverconv_CXXFLAGS = -Wall -W
rcgverconv_LDFLAGS = \
	-L$(top_builddir)/rcsc
rcgverconv_LDADD = -lrcsc

rcgversion_SOURCES = \
	rcgversion.cpp
rcgversion_CXXFLAGS = -Wall -W
rcgversion_LDFLAGS = \
	-L$(top_builddir)/rcsc
rcgversion_LDADD = -lrcsc

object_table_printer_SOURCES = \
	object_table_printer.cpp
object_table_printer_LDFLAGS = \
	-L$(top_builddir)/rcsc
object_table_printer_LDADD = -lrcsc

rcg_parser_bench_SOURCES = \
	rcg_parser_bench.cpp
rcg_parser_bench_LDFLAGS = \
	-L$(top_builddir)/rcsc
rcg_parser_bench_LDADD = -lrcsc

AM_CPPFLAGS = -I$(top_srcdir)
AM_CXXFLAGS = -Wall -W
AM_CFLAGS = -Wall -W
AM_LDFLAGS =

CLEANFILES = *~
