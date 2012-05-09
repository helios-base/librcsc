
if BUILD_EXAMPLE
EXAMPLE_PROGS = \
	test_result_writer \
	test_loader \
	test_gzifstream \
	test_gzofstream \
	test_param
endif

noinst_PROGRAMS = $(EXAMPLE_PROGS)

test_result_writer_SOURCES = \
	result_writer.cpp \
	result_writer_main.cpp

test_result_writer_LDFLAGS = -L$(top_builddir)/rcsc
test_result_writer_LDADD = -lrcsc_rcg -lrcsc_gz

test_loader_SOURCES = loader_main.cpp
test_loader_LDFLAGS = -L$(top_builddir)/rcsc
test_loader_LDADD = -lrcsc_rcg -lrcsc_gz

test_gzifstream_SOURCES = gzifstream_main.cpp
test_gzifstream_LDFLAGS = -L$(top_builddir)/rcsc
test_gzifstream_LDADD = -lrcsc_gz

test_gzofstream_SOURCES = gzofstream_main.cpp
test_gzofstream_LDFLAGS = -L$(top_builddir)/rcsc
test_gzofstream_LDADD = -lrcsc_gz

test_param_SOURCES = param_main.cpp
test_param_LDFLAGS = -L$(top_builddir)/rcsc
test_param_LDADD = -lrcsc_param

noinst_HEADERS = \
	result_writer.h

AM_CPPFLAGS = -I$(top_srcdir)
AM_CFLAGS = -Wall -W
AM_CXXFLAGS = -Wall -W
AM_LDFLAGS =

CLEANFILES = *~

#EXTRA_DIST =
