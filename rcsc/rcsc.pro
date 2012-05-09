TEMPLATE = lib
TARGET = rcsc
DESTDIR = ../lib
DEPENDPATH += . geom gz monitor param rcg
INCLUDEPATH += . .. ../boost ../zlib/include
#LIBS += -L../zlib -lz
LIBS += ../zlib/zlib1.dll
DEFINES += HAVE_LIBZ HAVE_WINDOWS_H
DEFINES += TRILIBRARY REDUCED CDT_ONLY NO_TIMER VOID=int REAL=double
CONFIG += staticlib warn_on release

# Input
HEADERS += factory.h \
           math_util.h \
           random.h \
           rcg.h \
           soccer_math.h \
           types.h \
           common/player_param.h \
           common/player_type.h \
           common/server_param.h \
           common/team_graphic.h \
           geom/triangle/triangle.h \
           geom/angle_deg.h \
           geom/circle_2d.h \
           geom/composite_region_2d.h \
           geom/delaunay_triangulation.h \
           geom/line_2d.h \
           geom/matrix_2d.h \
           geom/polygon_2d.h \
           geom/ray_2d.h \
           geom/rect_2d.h \
           geom/region_2d.h \
           geom/sector_2d.h \
           geom/segment_2d.h \
           geom/size_2d.h \
           geom/triangle_2d.h \
           geom/triangulation.h \
           geom/vector_2d.h \
           geom/voronoi_diagram.h \
           gz/gzfstream.h \
           monitor/monitor_command.h \
           trainer/trainer_command.h \
           param/cmd_line_parser.h \
           param/conf_file_parser.h \
           param/param_map.h \
           param/param_parser.h \
           param/rcss_param_parser.h \
           rcg/handler.h \
           rcg/holder.h \
           rcg/reader.h \
           rcg/parser.h \
           rcg/parser_v1.h \
           rcg/parser_v2.h \
           rcg/parser_v3.h \
           rcg/parser_v4.h \
           rcg/parser_v5.h \
           rcg/serializer.h \
           rcg/serializer_v1.h \
           rcg/serializer_v2.h \
           rcg/serializer_v3.h \
           rcg/serializer_v4.h \
           rcg/serializer_v5.h \
           rcg/types.h \
           rcg/util.h
SOURCES += common/player_param.cpp \
           common/player_type.cpp \
           common/server_param.cpp \
           common/team_graphic.cpp \
           geom/triangle/triangle.c \
           geom/angle_deg.cpp \
           geom/circle_2d.cpp \
           geom/composite_region_2d.cpp \
           geom/delaunay_triangulation.cpp \
           geom/line_2d.cpp \
           geom/matrix_2d.cpp \
           geom/polygon_2d.cpp \
           geom/ray_2d.cpp \
           geom/rect_2d.cpp \
           geom/sector_2d.cpp \
           geom/segment_2d.cpp \
           geom/triangle_2d.cpp \
           geom/triangulation.cpp \
           geom/vector_2d.cpp \
           geom/voronoi_diagram.cpp \
           gz/gzfstream.cpp \
           monitor/monitor_command.cpp \
           trainer/trainer_command.cpp \
           param/cmd_line_parser.cpp \
           param/conf_file_parser.cpp \
           param/param_map.cpp \
           param/rcss_param_parser.cpp \
           rcg/holder.cpp \
           rcg/parser.cpp \
           rcg/parser_v1.cpp \
           rcg/parser_v2.cpp \
           rcg/parser_v3.cpp \
           rcg/parser_v4.cpp \
           rcg/parser_v5.cpp \
           rcg/serializer.cpp \
           rcg/serializer_v1.cpp \
           rcg/serializer_v2.cpp \
           rcg/serializer_v3.cpp \
           rcg/serializer_v4.cpp \
           rcg/serializer_v5.cpp \
           rcg/util.cpp
