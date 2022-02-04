// -*-c++-*-

/*!
  \file coach_debug_client.h
  \brief interface for visual debug server Header File
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

#ifndef RCSC_COACH_COACH_DEBUG_CLIENT_H
#define RCSC_COACH_COACH_DEBUG_CLIENT_H

#include <rcsc/geom/segment_2d.h>
#include <rcsc/geom/triangle_2d.h>
#include <rcsc/geom/rect_2d.h>
#include <rcsc/geom/circle_2d.h>

#include <memory>
#include <fstream>
#include <string>

namespace rcsc {

class UDPSocket;
class CoachPlayerObject;
class CoachWorldModel;

class CoachDebugClient {
public:

    static const std::size_t MAX_LINE = 50; //!< maximum number of lines in one message.
    static const std::size_t MAX_TRIANGLE = 50; //!< maximum number of triangles in one message.
    static const std::size_t MAX_RECT = 50; //!< maximum number of rectangles in one message.
    static const std::size_t MAX_CIRCLE = 50; //!< maximum number of circles in one message.

private:

    struct Impl; //!< pimpl ideom

    Impl * M_impl;

    //! if false, all debug info are not created.
    bool M_on;

    //! flag to check connection.
    bool M_connected;

    //! connection to a debug server
    std::shared_ptr< UDPSocket > M_socket;

    //! output file stream
    std::ofstream M_server_log;

    //! flag to check write mode
    bool M_write_mode;

    //! main buffer to output all
    std::string M_main_buffer;

    //! target number shown in display
    int M_target_unum;
    //! target point shown in display
    Vector2D M_target_point;
    //! message shown in display
    std::string M_message;

public:

    /*!
      \brief init and/or reserve member variables
     */
    CoachDebugClient();

    /*!
      \brief close a connection and a stream.
     */
    ~CoachDebugClient();


    /*!
      \brief connect to the debug server
      \param hostname host name string thatdebug server is running
      \param port port number for debug server connection
    */
    bool connect( const std::string & hostname,
                  const int port );


    /*!
      \brief open debug server message log file.
      \param log_dir directry path name.
      \param teamname team name.
      \param unum player's uniform number.
     */
    bool open( const std::string & log_dir,
               const std::string & teamname );

    /*!
      \brief output to stream or socket
      \param world const reference to the world model instance
    */
    void writeAll( const CoachWorldModel & world );

private:
    /*!
      \brief close file and connection
    */
    void close();

    /*!
      \brief make debug message
      \param world const reference to the world mode instance
    */
    void buildString( const CoachWorldModel & world );

    /*!
      \brief send debug message to a debug server
    */
    void send();

    /*!
      \brief write debug message to file
      \param cycle current game cycle
    */
    void write( const long & cycle );

public:
    /*!
      \brief clear all data
    */
    void clear();

    /*!
      \brief add formated string to buffer
      \param msg formated text
    */
    void addMessage( const char * msg,
                     ... );

    /*!
      \brief add formated string to buffer
      \param msg formated text
    */
    void addMessage( const std::string & msg );

    /*!
      \brief add formated message for the player
      \param player pointer to the target player
     */
    void addComment( const CoachPlayerObject * player,
                     const char * msg,
                     ... );

    /*!
      \brief set target player
      \param unum target player's uniform number
    */
    void setTarget( const int unum )
      {
          M_target_unum = unum;
      }

    /*!
      \brief set target point
      \param p target point
    */
    void setTarget( const Vector2D & p )
      {
          M_target_point = p;
      }

    /*!
      \brief set line info to be drawn
      \param from line start point
      \param to line end point
    */
    void addLine( const Vector2D & from,
                  const Vector2D & to,
                  const char * color = "" );

    /*!
      \brief set triangle info to be drawn
      \param v1 vertex 1
      \param v2 vertex 2
      \param v3 vertex 3
    */
    void addTriangle( const Vector2D & v1,
                      const Vector2D & v2,
                      const Vector2D & v3,
                      const char * color = "" )
      {
          if ( M_on )
          {
              addTriangle( Triangle2D( v1, v2, v3 ), color );
          }
      }

    /*!
      \brief set triangle info to be drawn
      \param tri triangle object
    */
    void addTriangle( const Triangle2D & tri,
                      const char * color = "" );

    /*!
      \brief set rectangle info to be drawn
      \param rect rectanble object
    */
    void addRectangle( const Rect2D & rect,
                       const char * color = "" );

    /*!
      \brief set circle info to be drawn
      \param center center coordinate
      \param radius radius value
     */
    void addCircle( const Vector2D & center,
                    const double & radius,
                    const char * color = "" )
      {
          if ( M_on )
          {
              addCircle( Circle2D( center, radius ), color );
          }
      }

    /*!
      \brief set circle info to be drawn
      \param circle circle object
     */
    void addCircle( const Circle2D & circle,
                    const char * color = "" );
};

}

#endif
