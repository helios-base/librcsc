// -*-c++-*-

/*!
  \file logger.h
  \brief Logger class Header File
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifndef RCSC_PLAYER_LOGGER_H
#define RCSC_PLAYER_LOGGER_H

#include <rcsc/geom/vector_2d.h>
#include <rcsc/geom/circle_2d.h>
#include <rcsc/geom/rect_2d.h>
#include <rcsc/geom/sector_2d.h>
#include <rcsc/geom/triangle_2d.h>

#include <string>
#include <cstdio>
#include <cstdint>

namespace rcsc {

class GameTime;

/*!
  \class Logger
  \brief log output manager
*/
class Logger {
public:

    static const std::int32_t LEVEL_00 = 0x00000000; //!< log level definition variable
    static const std::int32_t LEVEL_01 = 0x00000001; //!< log level definition variable
    static const std::int32_t LEVEL_02 = 0x00000002; //!< log level definition variable
    static const std::int32_t LEVEL_03 = 0x00000004; //!< log level definition variable
    static const std::int32_t LEVEL_04 = 0x00000008; //!< log level definition variable
    static const std::int32_t LEVEL_05 = 0x00000010; //!< log level definition variable
    static const std::int32_t LEVEL_06 = 0x00000020; //!< log level definition variable
    static const std::int32_t LEVEL_07 = 0x00000040; //!< log level definition variable
    static const std::int32_t LEVEL_08 = 0x00000080; //!< log level definition variable
    static const std::int32_t LEVEL_09 = 0x00000100; //!< log level definition variable
    static const std::int32_t LEVEL_10 = 0x00000200; //!< log level definition variable
    static const std::int32_t LEVEL_11 = 0x00000400; //!< log level definition variable
    static const std::int32_t LEVEL_12 = 0x00000800; //!< log level definition variable
    static const std::int32_t LEVEL_13 = 0x00001000; //!< log level definition variable
    static const std::int32_t LEVEL_14 = 0x00002000; //!< log level definition variable
    static const std::int32_t LEVEL_15 = 0x00004000; //!< log level definition variable
    static const std::int32_t LEVEL_16 = 0x00008000; //!< log level definition variable
    static const std::int32_t LEVEL_17 = 0x00010000; //!< log level definition variable
    static const std::int32_t LEVEL_18 = 0x00020000; //!< log level definition variable
    static const std::int32_t LEVEL_19 = 0x00040000; //!< log level definition variable
    static const std::int32_t LEVEL_20 = 0x00080000; //!< log level definition variable
    static const std::int32_t LEVEL_21 = 0x00100000; //!< log level definition variable
    static const std::int32_t LEVEL_22 = 0x00200000; //!< log level definition variable
    static const std::int32_t LEVEL_23 = 0x00400000; //!< log level definition variable
    static const std::int32_t LEVEL_24 = 0x00800000; //!< log level definition variable
    static const std::int32_t LEVEL_25 = 0x01000000; //!< log level definition variable
    static const std::int32_t LEVEL_26 = 0x02000000; //!< log level definition variable
    static const std::int32_t LEVEL_27 = 0x04000000; //!< log level definition variable
    static const std::int32_t LEVEL_28 = 0x08000000; //!< log level definition variable
    static const std::int32_t LEVEL_29 = 0x10000000; //!< log level definition variable
    static const std::int32_t LEVEL_30 = 0x20000000; //!< log level definition variable
    static const std::int32_t LEVEL_31 = 0x40000000; //!< log level definition variable
    static const std::int32_t LEVEL_32 = 0x80000000; //!< log level definition variable

    /*************************************************
    Log Message Line Format:
    Line := <Time> <Level> <Type> <Content>
    Time := integer value
    Level := integer value
    Type :=  M | p | l | a | c | C | t | T | r | R | s | m
        M : log message for text viewer
        p : point
        l: line
        a: arc
        c: circle
        C: filled circle
        t: triangle
        T: filled triangle
        r: rectangle
        R: filled rectangle
        s: sector
        S: filled sector
        m: message painted on the field
    Text := <Str>
    Point := <x:Real> <y:Real>[ <Color>]
    Line := <x1:Real> <y1:Real> <x2:Real> <y2:Real>[ <Color>]
    Arc := <x:Real> <y:Real> <r:Real> <startAngle:Real> <spanAngle:Real>[ <Color>]
    Circle := <x:Real> <y:Real> <r:Real>[ <Color>]
    Triangle := <x1:Real> <y1:Real> <x2:Real> <y2:Real> <x3:Real> <y3:Real>[ <Color>]
    Rectangle := <leftX:Real> <topY:Real> <width:Real> <height:Real>[ <Color>]
    Sector := <x:Real> <y:Real> <minR:Real> <maxR:Real> <startAngle:Real> <spanAngle:Real>[ <Color>]
    Message := <x:Real> <y:Real>[ (c <Color>)] <Str>
    **************************************************/

    static const std::int32_t SYSTEM    = LEVEL_01; //!< log level definition alias
    static const std::int32_t SENSOR    = LEVEL_02; //!< log level definition alias
    static const std::int32_t WORLD     = LEVEL_03; //!< log level definition alias
    static const std::int32_t ACTION    = LEVEL_04; //!< log level definition alias
    static const std::int32_t INTERCEPT = LEVEL_05; //!< log level definition alias
    static const std::int32_t KICK      = LEVEL_06; //!< log level definition alias
    static const std::int32_t HOLD      = LEVEL_07; //!< log level definition alias
    static const std::int32_t DRIBBLE   = LEVEL_08; //!< log level definition alias
    static const std::int32_t PASS      = LEVEL_09; //!< log level definition alias
    static const std::int32_t CROSS     = LEVEL_10; //!< log level definition alias
    static const std::int32_t SHOOT     = LEVEL_11; //!< log level definition alias
    static const std::int32_t CLEAR     = LEVEL_12; //!< log level definition alias
    static const std::int32_t BLOCK     = LEVEL_13; //!< log level definition alias
    static const std::int32_t MARK      = LEVEL_14; //!< log level definition alias
    static const std::int32_t POSITIONING = LEVEL_15; //!< log level definition alias
    static const std::int32_t ROLE      = LEVEL_16; //!< log level definition alias
    static const std::int32_t TEAM      = LEVEL_17; //!< log level definition alias
    static const std::int32_t COMMUNICATION = LEVEL_18; //!< log level definition alias
    static const std::int32_t ANALYZER  = LEVEL_19; //!< log level definition alias
    static const std::int32_t ACT_SEQ   = LEVEL_20; //!< log level definition alias
    static const std::int32_t ACTION_CHAIN = ACT_SEQ; //!< log level definition alias
    static const std::int32_t PLAN      = LEVEL_21; //!< log level definition alias

    static const std::int32_t TRAINING  = LEVEL_32; //!< log level definition alias

    static const std::int32_t LEVEL_ANY = 0xffffffff; //!< log level definition variable

private:

    //! const pointer to GameTime instance
    const GameTime * M_time;

    //! output file stream
    FILE * M_fout;

    //! log level flag
    std::int32_t M_flags;

    int M_start_time;
    int M_end_time;

public:
    /*!
      \brief allocate message buffer memory
     */
    Logger();

    /*!
      \brief if file is opened, flush buffer and close file.
     */
    ~Logger();

    /*!
      \brief set new log level
      \param time const pointer to the game time instance
      \param level log level
      \param on if true, set flag for the level
     */
    void setLogFlag( const GameTime * time,
                     const std::int32_t level,
                     const bool on = true );

    /*!
      \brief set the range of the recorded game time
      \param start_time lower value
      \param end_time higher value
     */
    void setTimeRange( const int start_time,
                       const int end_time );

    /*!
      \brief check if the level is enabled
      \param level checked log level
      \return true if the level is enabled or not
     */
    bool isEnabled( const std::int32_t level ) const
      {
          return ( level & M_flags );
      }

    /*!
      \brief open file to record
      \param filepath file path string
     */
    void open( const std::string & filepath );

    /*!
      \brief use standard output to record
     */
    void openStandardOutput();

    /*!
      \brief use standard error to record
     */
    void openStandardError();

    /*!
      \brief close file if opened
     */
    void close();

    /*!
      \brief check if file is opened
      \return true if file is opened
     */
    bool isOpen()
      {
          return ( M_fout != NULL );
      }

    /*!
      \brief flush stored message
    */
    void flush();

    /*!
      \brief clear buffer without flush
    */
    void clear();

    /*!
      \brief add free message to buffer with cycle, level & message tag 'T'
      \param level debug flag level
      \param msg message
     */
    void addText( const std::int32_t level,
                  const char * msg,
                  ... );

    /*!
      \brief add point info to buffer with cycle, level & message tag 'p'
      \param level log level variable
      \param x point coordinate x
      \param y point coordinate y
      \param color color name string
     */
    void addPoint( const std::int32_t level,
                   const double x,
                   const double y,
                   const char * color = NULL );

    /*!
      \brief add point info to buffer with cycle, level & message tag 'p'
      \param level log level variable
      \param pos point coordinate
      \param color color name string
     */
    void addPoint( const std::int32_t level,
                   const Vector2D & pos,
                   const char * color = NULL )
      {
          addPoint( level, pos.x, pos.y, color );
      }

    /*!
      \brief add point info to buffer with cycle, level & message tag 'p'
      \param level log level variable
      \param x point coordinate x
      \param y point coordinate y
      \param r red value
      \param g green value
      \param b blue value
     */
    void addPoint( const std::int32_t level,
                   const double x,
                   const double y,
                   const int r, const int g, const int b );

    /*!
      \brief add point info to buffer with cycle, level & message tag 'p'
      \param level log level variable
      \param pos point coordinate
      \param r red value
      \param g green value
      \param b blue value
     */
    void addPoint( const std::int32_t level,
                   const Vector2D & pos,
                   const int r, const int g, const int b )
      {
          addPoint( level, pos.x, pos.y, r, g, b );
      }

    /*!
      \brief add line info to buffer with cycle, level & message tag 'l'
      \param level log level variable
      \param x1 line start point coordinate x
      \param y1 line start point coordinate y
      \param x2 line end point coordinate x
      \param y2 line end point coordinate y
      \param color color name string
     */
    void addLine( const std::int32_t level,
                  const double x1,
                  const double y1,
                  const double x2,
                  const double y2,
                  const char * color = NULL );

    /*!
      \brief add line info to buffer with cycle, level & message tag 'l'
      \param level log level variable
      \param start line start point coordinate
      \param end line end point coordinate
      \param color color name string
     */
    void addLine( const std::int32_t level,
                  const Vector2D & start,
                  const Vector2D & end,
                  const char * color = NULL )
      {
          addLine( level, start.x, start.y, end.x, end.y, color );
      }

    /*!
      \brief add line info to buffer with cycle, level & message tag 'l'
      \param level log level variable
      \param x1 line start point coordinate x
      \param y1 line start point coordinate y
      \param x2 line end point coordinate x
      \param y2 line end point coordinate y
      \param r red value
      \param g green value
      \param b blue value
     */
    void addLine( const std::int32_t level,
                  const double x1,
                  const double y1,
                  const double x2,
                  const double y2,
                  const int r, const int g, const int b );

    /*!
      \brief add line info to buffer with cycle, level & message tag 'l'
      \param level log level variable
      \param start line start point coordinate
      \param end line end point coordinate
      \param r red value
      \param g green value
      \param b blue value
     */
    void addLine( const std::int32_t level,
                  const Vector2D & start,
                  const Vector2D & end,
                  const int r, const int g, const int b )
      {
          addLine( level, start.x, start.y, end.x, end.y, r, g, b );
      }

    /*!
      \brief add arc info to the buffer. message tag 'a'
      \param level log level value
      \param x center x of the circle
      \param y center y of the circle
      \param radius radius value of the circle
      \param start_angle start angle
      \param span_angle span of the arc. direction is clockwise from start_angle
      \param color color name string
     */
    void addArc( const std::int32_t level,
                 const double x,
                 const double y,
                 const double radius,
                 const AngleDeg & start_angle,
                 const double span_angle,
                 const char * color = NULL );

    /*!
      \brief add arc info to the buffer. message tag 'a'
      \param level log level value
      \param center center of the circle
      \param radius radius value of the circle
      \param start_angle start angle
      \param span_angle span of the arc. direction is clockwise from start_angle
      \param color color name string
     */
    void addArc( const std::int32_t level,
                 const Vector2D & center,
                 const double radius,
                 const AngleDeg & start_angle,
                 const double span_angle,
                 const char * color = NULL )
      {
          addArc( level, center.x, center.y, radius, start_angle, span_angle, color );
      }

    /*!
      \brief add arc info to the buffer. message tag 'a'
      \param level log level value
      \param x center x of the circle
      \param y center y of the circle
      \param radius radius value of the circle
      \param start_angle start angle
      \param span_angle span of the arc. direction is clockwise from start_angle
      \param r red value
      \param g green value
      \param b blue value
     */
    void addArc( const std::int32_t level,
                 const double x,
                 const double y,
                 const double radius,
                 const AngleDeg & start_angle,
                 const double span_angle,
                 const int r, const int g, const int b );

    /*!
      \brief add arc info to the buffer. message tag 'a'
      \param level log level value
      \param center center of the circle
      \param radius radius value of the circle
      \param start_angle start angle
      \param span_angle span of the arc. direction is clockwise from start_angle
      \param r red value
      \param g green value
      \param b blue value
     */
    void addArc( const std::int32_t level,
                 const Vector2D & center,
                 const double radius,
                 const AngleDeg & start_angle,
                 const double span_angle,
                 const int r, const int g, const int b )
      {
          addArc( level, center.x, center.y, radius, start_angle, span_angle, r, g, b );
      }

    /*!
      \brief add circle info to buffer with cycle, level & message tag 'c'
      \param level log level variable
      \param x circle center point coordinate x
      \param y circle center point coordinate y
      \param radius circle radius
      \param color color name string
      \param fill switch to select a circle is filled or not.
     */
    void addCircle( const std::int32_t level,
                    const double x,
                    const double y,
                    const double radius,
                    const char * color = NULL,
                    const bool fill = false );

    /*!
      \brief add circle info to buffer with cycle, level & message tag 'c'
      \param level log level variable
      \param center circle center point coordinate
      \param radius circle radius
      \param color color name string
      \param fill switch to select a circle is filled or not.
     */
    void addCircle( const std::int32_t level,
                    const Vector2D & center,
                    const double radius,
                    const char * color = NULL,
                    const bool fill = false )
      {
          addCircle( level, center.x, center.y, radius, color, fill );
      }

    /*!
      \brief add circle info to buffer with cycle, level & message tag 'c'
      \param level log level variable
      \param circle circle object
      \param color color name string
      \param fill switch to select a circle is filled or not.
     */
    void addCircle( const std::int32_t level,
                    const Circle2D & circle,
                    const char * color = NULL,
                    const bool fill = false )
      {
          addCircle( level, circle.center().x, circle.center().y, circle.radius(), color, fill );
      }

    /*!
      \brief add circle info to buffer with cycle, level & message tag 'c'
      \param level log level variable
      \param x circle center point coordinate x
      \param y circle center point coordinate y
      \param radius circle radius
      \param r red value
      \param g green value
      \param b blue value
      \param fill switch to select a circle is filled or not.
     */
    void addCircle( const std::int32_t level,
                    const double x,
                    const double y,
                    const double radius,
                    const int r, const int g, const int b,
                    const bool fill = false );

    /*!
      \brief add circle info to buffer with cycle, level & message tag 'c'
      \param level log level variable
      \param center circle center point coordinate
      \param radius circle radius
      \param r red value
      \param g green value
      \param b blue value
      \param fill switch to select a circle is filled or not.
     */
    void addCircle( const std::int32_t level,
                    const Vector2D & center,
                    const double radius,
                    const int r, const int g, const int b,
                    const bool fill = false )
      {
          addCircle( level, center.x, center.y, radius, r, g, b, fill );
      }

    /*!
      \brief add circle info to buffer with cycle, level & message tag 'c'
      \param level log level variable
      \param circle circle object
      \param r red value
      \param g green value
      \param b blue value
      \param fill switch to select a circle is filled or not.
     */
    void addCircle( const std::int32_t level,
                    const Circle2D & circle,
                    const int r, const int g, const int b,
                    const bool fill = false )
      {
          addCircle( level,
                     circle.center().x, circle.center().y, circle.radius(),
                     r, g, b,
                     fill );
      }

    /*!
      \brief add triangle info to buffer with cycle, level & message tag 't'
      \param level log level variable
      \param x1 1st vertex coordinate x
      \param y1 1st vertex coordinate y
      \param x2 2nd vertex coordinate x
      \param y2 2nd vertex coordinate y
      \param x3 3rd vertex coordinate x
      \param y3 3rd vertex coordinate y
      \param color color name string
      \param fill switch to select a circle is filled or not.
     */
    void addTriangle( const std::int32_t level,
                      const double x1,
                      const double y1,
                      const double x2,
                      const double y2,
                      const double x3,
                      const double y3,
                      const char * color = NULL,
                      const bool fill = false );

    /*!
      \brief add triangle info to buffer with cycle, level & message tag 't'
      \param level log level variable
      \param p1 1st vertex coordinate
      \param p2 2nd vertex coordinate
      \param p3 3rd vertex coordinate
      \param color color name string
      \param fill switch to select a circle is filled or not.
     */
    void addTriangle( const std::int32_t level,
                      const Vector2D & p1,
                      const Vector2D & p2,
                      const Vector2D & p3,
                      const char * color = NULL,
                      const bool fill = false )
      {
          addTriangle( level,
                       p1.x, p1.y,
                       p2.x, p2.y,
                       p3.x, p3.y,
                       color,
                       fill );
      }

    /*!
      \brief add triangle info to buffer with cycle, level & message tag 't'
      \param level log level variable
      \param tri triangle object
      \param color color name string
      \param fill switch to select a circle is filled or not.
     */
    void addTriangle( const std::int32_t level,
                      const Triangle2D & tri,
                      const char * color = NULL,
                      const bool fill = false )
      {
          addTriangle( level,
                       tri.a().x, tri.a().y,
                       tri.b().x, tri.b().y,
                       tri.c().x, tri.c().y,
                       color,
                       fill );
      }

    /*!
      \brief add triangle info to buffer with cycle, level & message tag 't'
      \param level log level variable
      \param x1 1st vertex coordinate x
      \param y1 1st vertex coordinate y
      \param x2 2nd vertex coordinate x
      \param y2 2nd vertex coordinate y
      \param x3 3rd vertex coordinate x
      \param y3 3rd vertex coordinate y
      \param r red value
      \param g green value
      \param b blue value
      \param fill switch to select a circle is filled or not.
     */
    void addTriangle( const std::int32_t level,
                      const double x1,
                      const double y1,
                      const double x2,
                      const double y2,
                      const double x3,
                      const double y3,
                      const int r, const int g, const int b,
                      const bool fill = false );

    /*!
      \brief add triangle info to buffer with cycle, level & message tag 't'
      \param level log level variable
      \param p1 1st vertex coordinate
      \param p2 2nd vertex coordinate
      \param p3 3rd vertex coordinate
      \param r red value
      \param g green value
      \param b blue value
      \param fill switch to select a circle is filled or not.
     */
    void addTriangle( const std::int32_t level,
                      const Vector2D & p1,
                      const Vector2D & p2,
                      const Vector2D & p3,
                      const int r, const int g, const int b,
                      const bool fill = false )
      {
          addTriangle( level,
                       p1.x, p1.y,
                       p2.x, p2.y,
                       p3.x, p3.y,
                       r, g, b,
                       fill );
      }

    /*!
      \brief add triangle info to buffer with cycle, level & message tag 't'
      \param level log level variable
      \param tri triangle object
      \param r red value
      \param g green value
      \param b blue value
      \param fill switch to select a circle is filled or not.
     */
    void addTriangle( const std::int32_t level,
                      const Triangle2D & tri,
                      const int r, const int g, const int b,
                      const bool fill = false )
      {
          addTriangle( level,
                       tri.a().x, tri.a().y,
                       tri.b().x, tri.b().y,
                       tri.c().x, tri.c().y,
                       r, g, b,
                       fill );
      }

    /*!
      \brief add rect info to buffer with cycle, level & message tag 'r'
      \param level log level variable
      \param left top left point coordinate x
      \param top top left point coordinate y
      \param length x range of the rectangle
      \param width y range of the rectangle
      \param color color name string
      \param fill switch to select a circle is filled or not.
     */
    void addRect( const std::int32_t level,
                  const double left,
                  const double top,
                  const double length,
                  const double width,
                  const char * color = NULL,
                  const bool fill = false );

    /*!
      \brief add rect info to buffer with cycle, level & message tag 'r'
      \param level log level variable
      \param rect rectangle object
      \param color color name string
      \param fill switch to select a circle is filled or not.
     */
    void addRect( const std::int32_t level,
                  const Rect2D & rect,
                  const char * color = NULL,
                  const bool fill = false )
      {
          addRect( level,
                   rect.left(), rect.top(),
                   rect.size().length(), rect.size().width(),
                   color,
                   fill );
      }

    /*!
      \brief add rect info to buffer with cycle, level & message tag 'r'
      \param level log level variable
      \param left top left point coordinate x
      \param top top left point coordinate y
      \param length x range of the rectangle
      \param width y range of the rectangle
      \param r red value
      \param g green value
      \param b blue value
      \param fill switch to select a circle is filled or not.
     */
    void addRect( const std::int32_t level,
                  const double left,
                  const double top,
                  const double length,
                  const double width,
                  const int r, const int g, const int b,
                  const bool fill = false );

    /*!
      \brief add rect info to buffer with cycle, level & message tag 'r'
      \param level log level variable
      \param rect rectangle object
      \param r red value
      \param g green value
      \param b blue value
      \param fill switch to select a circle is filled or not.
     */
    void addRect( const std::int32_t level,
                  const Rect2D & rect,
                  const int r, const int g, const int b,
                  const bool fill = false )
      {
          addRect( level,
                   rect.left(), rect.top(),
                   rect.size().length(), rect.size().width(),
                   r, g, b,
                   fill );
      }

    /*!
      \brief add arc info to the buffer. message tag 'a'
      \param level log level value
      \param x center x of the circle
      \param y center y of the circle
      \param min_radius min radius value of the circle
      \param max_radius min radius value of the circle
      \param start_angle start angle
      \param span_angle span of the arc. direction is clockwise from start_angle
      \param color color name string
      \param fill switch to select a sector is filled or not
     */
    void addSector( const std::int32_t level,
                    const double x,
                    const double y,
                    const double min_radius,
                    const double max_radius,
                    const AngleDeg & start_angle,
                    const double span_angle,
                    const char * color = NULL,
                    const bool fill = false );

    /*!
      \brief add arc info to the buffer. message tag 'a'
      \param level log level value
      \param center center of the circle
      \param min_radius min radius value of the circle
      \param max_radius min radius value of the circle
      \param start_angle start angle
      \param span_angle span of the arc. direction is clockwise from start_angle
      \param color color name string
      \param fill switch to select a sector is filled or not
     */
    void addSector( const std::int32_t level,
                    const Vector2D & center,
                    const double min_radius,
                    const double max_radius,
                    const AngleDeg & start_angle,
                    const double span_angle,
                    const char * color = NULL,
                    const bool fill = false )
      {
          addSector( level,
                     center.x, center.y, min_radius, max_radius,
                     start_angle, span_angle,
                     color,
                     fill );
      }

    /*!
      \brief add arc info to the buffer. message tag 'a'
      \param level log level value
      \param x center x of the circle
      \param y center y of the circle
      \param min_radius min radius value of the circle
      \param max_radius min radius value of the circle
      \param start_angle start angle
      \param span_angle span of the arc. direction is clockwise from start_angle
      \param r red value
      \param g green value
      \param b blue value
      \param fill switch to select a sector is filled or not
     */
    void addSector( const std::int32_t level,
                    const double x,
                    const double y,
                    const double min_radius,
                    const double max_radius,
                    const AngleDeg & start_angle,
                    const double span_angle,
                    const int r, const int g, const int b,
                    const bool fill = false );

    /*!
      \brief add arc info to the buffer. message tag 'a'
      \param level log level value
      \param center center of the circle
      \param min_radius min radius value of the circle
      \param max_radius min radius value of the circle
      \param start_angle start angle
      \param span_angle span of the arc. direction is clockwise from start_angle
      \param r red value
      \param g green value
      \param b blue value
      \param fill switch to select a sector is filled or not
     */
    void addSector( const std::int32_t level,
                    const Vector2D & center,
                    const double min_radius,
                    const double max_radius,
                    const AngleDeg & start_angle,
                    const double span_angle,
                    const int r, const int g, const int b,
                    const bool fill = false )
      {
          addSector( level,
                     center.x, center.y, min_radius, max_radius,
                     start_angle, span_angle,
                     r, g, b,
                     fill );
      }

    /*!
      \brief add arc info to the buffer. message tag 'a'
      \param level log level value
      \param sector sector object
      \param color color name string
      \param fill switch to select a sector is filled or not
     */
    void addSector( const std::int32_t level,
                    const Sector2D & sector,
                    const char * color = NULL,
                    const bool fill = false );

    /*!
      \brief add arc info to the buffer. message tag 'a'
      \param level log level value
      \param sector sector object
      \param r red value
      \param g green value
      \param b blue value
      \param fill switch to select a sector is filled or not
     */
    void addSector( const std::int32_t level,
                    const Sector2D & sector,
                    const int r, const int g, const int b,
                    const bool fill = false );

    /*!
      \brief add message info to buffer with cycle, level & message tag 'm'
      \param level log level variable
      \param x painted point coordinate x
      \param y painted point coordinate y
      \param msg painted text
      \param color color name string
     */
    void addMessage( const std::int32_t level,
                     const double x,
                     const double y,
                     const char * msg,
                     const char * color = NULL );

    /*!
      \brief add message info to buffer with cycle, level & message tag 'm'
      \param level log level variable
      \param pos painted point coordinate
      \param msg drawd text
      \param color color name string
     */
    void addMessage( const std::int32_t level,
                     const Vector2D & pos,
                     const char * msg,
                     const char * color = NULL )
      {
          addMessage( level,
                      pos.x, pos.y, msg,
                      color );
      }

    /*!
      \brief add message info to buffer with cycle, level & message tag 'm'
      \param level log level variable
      \param x painted point coordinate x
      \param y painted point coordinate y
      \param msg painted text
      \param r red value
      \param g green value
      \param b blue value
     */
    void addMessage( const std::int32_t level,
                     const double x,
                     const double y,
                     const char * msg,
                     const int r, const int g, const int b );

    /*!
      \brief add message info to buffer with cycle, level & message tag 'm'
      \param level log level variable
      \param pos painted point coordinate
      \param msg painted text
      \param r red value
      \param g green value
      \param b blue value
     */
    void addMessage( const std::int32_t level,
                     const Vector2D & pos,
                     const char * msg,
                     const int r, const int g, const int b )
      {
          addMessage( level,
                      pos.x, pos.y, msg,
                      r, g, b );
      }

};

//! global variable
extern Logger dlog;

}

#endif
