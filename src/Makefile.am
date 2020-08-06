
bin_PROGRAMS = \
	rclmscheduler \
	rclmtableprinter \
	rcg2txt \
	rcgrenameteam \
	rcgresultprinter \
	rcgreverse \
	rcgverconv \
	rcgversion


rclmscheduler_SOURCES = \
	scheduler.cpp
rclmscheduler_CXXFLAGS = -Wall -W
rclmscheduler_LDFLAGS =
rclmscheduler_LDADD =

rcgresultprinter_SOURCES = \
	resultprinter.cpp
rcgresultprinter_CXXFLAGS = -Wall -W
rcgresultprinter_LDFLAGS = \
	-L$(top_builddir)/rcsc \
	-L$(top_builddir)/rcsc
rcgresultprinter_LDADD = -lrcsc

rclmtableprinter_SOURCES = \
	tableprinter.cpp
rclmtableprinter_CXXFLAGS = -Wall -W
rclmtableprinter_LDFLAGS =
rclmtableprinter_LDADD =


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

rcgverconv_SOURCES = \
	rcgverconv.cpp
rcgversion_CXXFLAGS = -Wall -W
rcgverconv_LDFLAGS = \
	-L$(top_builddir)/rcsc
rcgverconv_LDADD = -lrcsc

rcgversion_SOURCES = \
	rcgversion.cpp
rcgversion_LDFLAGS = \
	-L$(top_builddir)/rcsc
rcgversion_LDADD = -lrcsc

AM_CPPFLAGS = -I$(top_srcdir)
AM_CXXFLAGS = -Wall -W
AM_CFLAGS = -Wall -W
AM_LDFLAGS =

CLEANFILES = *~
