## Process this file with automake to produce Makefile.in

SUBDIRS = triangle .

noinst_LTLIBRARIES = librcsc_geom.la
#lib_LTLIBRARIES = librcsc_geom.la

librcsc_geom_la_SOURCES = \
	angle_deg.cpp \
	circle_2d.cpp \
	composite_region_2d.cpp \
	convex_hull.cpp \
	delaunay_triangulation.cpp \
	line_2d.cpp \
	matrix_2d.cpp \
	polygon_2d.cpp \
	ray_2d.cpp \
	rect_2d.cpp \
	sector_2d.cpp \
	segment_2d.cpp \
	triangle_2d.cpp \
	triangulation.cpp \
	vector_2d.cpp \
	voronoi_diagram.cpp \
	voronoi_diagram_triangle.cpp

#	segment_intersection.cpp


librcsc_geomincludedir = $(includedir)/rcsc/geom

##pkginclude_HEADERS
librcsc_geominclude_HEADERS = \
	angle_deg.h \
	circle_2d.h \
	composite_region_2d.h \
	convex_hull.h \
	delaunay_triangulation.h \
	line_2d.h \
	matrix_2d.h \
	polygon_2d.h \
	ray_2d.h \
	rect_2d.h \
	region_2d.h \
	sector_2d.h \
	size_2d.h \
	segment_2d.h \
	triangle_2d.h \
	triangulation.h \
	vector_2d.h \
	voronoi_diagram.h \
	voronoi_diagram_triangle.h

#	segment_intersection.h


librcsc_geom_la_LIBADD = \
	triangle/librcsc_geom_triangle.la

#	qhull/src/libqhull.la
#	tetgen/librcsc_geom_tetgen.la


librcsc_geom_la_LDFLAGS = -version-info 8:0:1
#libXXXX_la_LDFLAGS = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
#    1. Start with version information of `0:0:0' for each libtool library.
#
#    2. Update the version information only immediately before a public
#       release of your software.  More frequent updates are unnecessary,
#       and only guarantee that the current interface number gets larger
#       faster.
#
#    3. If the library source code has changed at all since the last
#       update, then increment REVISION (`C:R:A' becomes `C:r+1:A').
#
#    4. If any interfaces have been added, removed, or changed since the
#       last update, increment CURRENT, and set REVISION to 0.
#
#    5. If any interfaces have been added since the last public release,
#       then increment AGE.
#
#    6. If any interfaces have been removed since the last public release,
#       then set AGE to 0.

AM_CPPFLAGS = -I$(top_srcdir)
AM_CFLAGS = -Wall -W
AM_CXXFLAGS = -Wall -W
AM_LDFLAGS =

CLEANFILES = points.dat edges.dat *~

if UNIT_TEST
TESTS = \
	run_test_vector_2d \
	run_test_matrix_2d \
	run_test_segment_2d \
	run_test_triangle_2d \
	run_test_rect_2d \
	run_test_polygon_2d \
	run_test_voronoi_diagram \
	run_test_convex_hull \
	rundom_convex_hull
endif

check_PROGRAMS = $(TESTS)

run_test_vector_2d_SOURCES = test_vector_2d.cpp
run_test_vector_2d_CXXFLAGS = $(CPPUNIT_CFLAGS) -Wall -W
run_test_vector_2d_LDFLAGS = -L$(top_builddir)/rcsc/geom
run_test_vector_2d_LDADD = -lrcsc_geom $(CPPUNIT_LIBS)

run_test_matrix_2d_SOURCES = test_matrix_2d.cpp
run_test_matrix_2d_CXXFLAGS = $(CPPUNIT_CFLAGS) -Wall -W
run_test_matrix_2d_LDFLAGS = -L$(top_builddir)/rcsc/geom
run_test_matrix_2d_LDADD = -lrcsc_geom $(CPPUNIT_LIBS)

run_test_segment_2d_SOURCES = test_segment_2d.cpp
run_test_segment_2d_CXXFLAGS = $(CPPUNIT_CFLAGS) -Wall -W
run_test_segment_2d_LDFLAGS = -L$(top_builddir)/rcsc/geom
run_test_segment_2d_LDADD = -lrcsc_geom $(CPPUNIT_LIBS)

run_test_triangle_2d_SOURCES = test_triangle_2d.cpp
run_test_triangle_2d_CXXFLAGS = $(CPPUNIT_CFLAGS) -Wall -W
run_test_triangle_2d_LDFLAGS = -L$(top_builddir)/rcsc/geom
run_test_triangle_2d_LDADD = -lrcsc_geom $(CPPUNIT_LIBS)

run_test_rect_2d_SOURCES = test_rect_2d.cpp
run_test_rect_2d_CXXFLAGS = $(CPPUNIT_CFLAGS) -Wall -W
run_test_rect_2d_LDFLAGS = -L$(top_builddir)/rcsc/geom
run_test_rect_2d_LDADD = -lrcsc_geom $(CPPUNIT_LIBS)

run_test_polygon_2d_SOURCES = test_polygon_2d.cpp
run_test_polygon_2d_CXXFLAGS = $(CPPUNIT_CFLAGS) -Wall -W
run_test_polygon_2d_LDFLAGS = -L$(top_builddir)/rcsc/geom
run_test_polygon_2d_LDADD = -lrcsc_geom $(CPPUNIT_LIBS)

run_test_voronoi_diagram_SOURCES = test_voronoi_diagram.cpp
run_test_voronoi_diagram_CXXFLAGS = $(CPPUNIT_CFLAGS) -Wall -W
run_test_voronoi_diagram_LDFLAGS = -L$(top_builddir)/rcsc/geom
run_test_voronoi_diagram_LDADD = -lrcsc_geom $(CPPUNIT_LIBS)

run_test_convex_hull_SOURCES = test_convex_hull.cpp
run_test_convex_hull_CXXFLAGS = $(CPPUNIT_CFLAGS) -Wall -W
run_test_convex_hull_LDFLAGS = -L$(top_builddir)/rcsc/geom -L$(top_builddir)/rcsc/time
run_test_convex_hull_LDADD = -lrcsc_geom -lrcsc_time $(CPPUNIT_LIBS)

rundom_convex_hull_SOURCES = test_rundom_convex_hull.cpp
rundom_convex_hull_CXXFLAGS = $(CPPUNIT_CFLAGS) -Wall -W
rundom_convex_hull_LDFLAGS = -L$(top_builddir)/rcsc/geom
rundom_convex_hull_LDADD = -lrcsc_geom $(CPPUNIT_LIBS)


## noinst_PROGRAMS = \
## 	run_test_qhull_delaunay \
## 	run_test_qhull_voronoi
##
## run_test_qhull_delaunay_SOURCES = test_qhull_delaunay.cpp
## run_test_qhull_delaunay_CXXFLAGS = $(CPPUNIT_CFLAGS) -Wall -W
## run_test_qhull_delaunay_LDFLAGS = -L$(top_builddir)/rcsc/geom/qhull/src
## run_test_qhull_delaunay_LDADD = -lrcsc_geom $(CPPUNIT_LIBS)
##
## run_test_qhull_voronoi_SOURCES = test_qhull_voronoi.cpp
## run_test_qhull_voronoi_CXXFLAGS = $(CPPUNIT_CFLAGS) -Wall -W
## run_test_qhull_voronoi_LDFLAGS = -L$(top_builddir)/rcsc/geom/qhull/src
## run_test_qhull_voronoi_LDADD = -lrcsc_geom $(CPPUNIT_LIBS)
