// -*-c++-*-

/*!
  \file coach_debug_client.cpp
  \brief interface for visual debug server Source File
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "coach_debug_client.h"

#include "coach_world_model.h"

#include <rcsc/net/udp_socket.h>
#include <rcsc/types.h>

#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#include <cstdio>
#include <cstdarg>
#include <cmath>

namespace rcsc {

// accessible only from this file
namespace {

//! max send buffer size for debug client
#define G_BUFFER_SIZE 8192*4

//! global variable
static char g_buffer[G_BUFFER_SIZE];


//! rounding utility
inline
double
ROUND( const double & val,
       const double & step )
{
    return rint( val / step ) * step;
}

/*!
  \struct LineT
  \brief draw info
*/
struct LineT {
    Segment2D line_; //!< line segment data
    std::string color_; //!< color name or #RGB

    /*!
      \brief construct with values
      \param l line segment
      \param color color name or #RGB
    */
    LineT( const Segment2D & l,
           const char * color )
        : line_( l ),
          color_( color )
      { }
};

/*!
  \struct TriangleT
  \brief draw info
*/
struct TriangleT {
    Triangle2D triangle_; //!< triangle data
    std::string color_; //!< color name or #RGB

    /*!
      \brief construct with values
      \param t triangle
      \param color color name or #RGB
    */
    TriangleT( const Triangle2D & t,
               const char * color )
        : triangle_( t ),
          color_( color )
      { }
};

/*!
  \struct RectangleT
  \brief draw info
*/
struct RectangleT {
    Rect2D rect_; //!< rectangle data
    std::string color_; //!< color name or #RGB

    /*!
      \brief construct with values
      \param r rectangle
      \param color color name or #RGB
    */
    RectangleT( const Rect2D & r,
                const char * color )
        : rect_( r ),
          color_( color )
      { }
};

/*!
  \struct CircleT
  \brief draw info
*/
struct CircleT {
    Circle2D circle_; //!< circle data
    std::string color_; //!< color name or #RGB

    /*!
      \brief construct with values
      \param c circle
      \param color color name or #RGB
    */
    CircleT( const Circle2D & c,
             const char * color )
        : circle_( c ),
          color_( color )
      { }
};

/*-------------------------------------------------------------------*/

class PlayerPrinter {
private:
    std::ostream & M_os;
    const char M_tag;
    const std::map< const CoachPlayerObject *, std::string > & message_map_;
public:
    PlayerPrinter( std::ostream & os,
                   const char tag,
                   const std::map< const CoachPlayerObject *, std::string > & message_map )
        : M_os( os ),
          M_tag( tag ),
          message_map_( message_map )
      { }

    void operator()( const CoachPlayerObject * p )
      {
          M_os << " (";
          M_os << M_tag << ' ' << p->unum() << ' ' << p->type();

          M_os << ' ' << ROUND( p->pos().x, 0.01 )
               << ' ' << ROUND( p->pos().y, 0.01 );
          M_os << " (bd " << rint( p->body().degree() ) << ')';
          if ( p->pointtoCycle() > 0 )
          {
              M_os << " (pt " << rint( p->pointtoAngle().degree() ) << ')';
          }

          M_os << " (c \"";

          if ( M_tag == 'o' )
          {
              M_os << 'h' << p->type();
          }

          if  ( p->goalie() )
          {
              M_os << 'G';
          }

          if ( M_tag == 't' )
          {
              M_os << 'r' << ROUND( p->recovery(), 0.01 );
          }

          if ( p->isPointing() )
          {
              M_os << 'p'  << p->pointtoCycle() << ','
                   << rint( p->pointtoAngle().degree() );
          }

          if ( p->isTackling() )
          {
              M_os << 't' << p->tackleCycle();
          }
          else if ( p->isKicking() )
          {
              M_os << 'k';
          }
          else if ( p->isCharged() )
          {
              M_os << 'f' << p->chargedCycle();
          }

          if ( p->card() == YELLOW )
          {
              M_os << 'y';
          }

          std::map< const CoachPlayerObject *, std::string >::const_iterator it = message_map_.find( p );
          if ( it != message_map_.end() )
          {
              M_os << '|' << it->second;
          }

          M_os << "\"))";
      }
};

/*-------------------------------------------------------------------*/

class LinePrinter {
private:
    std::ostream & M_os;
public:
    LinePrinter( std::ostream & os )
        : M_os( os )
      { }
    void operator()( const LineT & line )
      {
          M_os << " (line "
               << ROUND( line.line_.origin().x, 0.001 ) << ' '
               << ROUND( line.line_.origin().y, 0.001 ) << ' '
               << ROUND( line.line_.terminal().x, 0.001 ) << ' '
               << ROUND( line.line_.terminal().y, 0.001 );
          if ( ! line.color_.empty() )
          {
              M_os << " \"" << line.color_ << '"';
          }
          M_os << ')';
      }
};


class TrianglePrinter {
private:
    std::ostream & M_os;
public:
    TrianglePrinter( std::ostream & os )
        : M_os( os )
      { }
    void operator()( const TriangleT & tri )
      {
          M_os << " (tri "
               << ROUND( tri.triangle_.a().x, 0.001 ) << ' '
               << ROUND( tri.triangle_.a().y, 0.001 ) << ' '
               << ROUND( tri.triangle_.b().x, 0.001 ) << ' '
               << ROUND( tri.triangle_.b().y, 0.001 ) << ' '
               << ROUND( tri.triangle_.c().x, 0.001 ) << ' '
               << ROUND( tri.triangle_.c().y, 0.001 );
          if ( ! tri.color_.empty() )
          {
              M_os << " \"" << tri.color_ << '"';
          }
          M_os << ')';
      }
};

class RectPrinter {
private:
    std::ostream & M_os;
public:
    RectPrinter( std::ostream & os )
        : M_os( os )
      { }
    void operator()( const RectangleT & rect )
      {
          M_os << " (rect "
               << ROUND( rect.rect_.left(), 0.001 ) << ' '
               << ROUND( rect.rect_.top(), 0.001 ) << ' '
               << ROUND( rect.rect_.right(), 0.001 ) << ' '
               << ROUND( rect.rect_.bottom(), 0.001 );
          if ( ! rect.color_.empty() )
          {
              M_os << " \"" << rect.color_ << '"';
          }
          M_os << ')';
      }
};

class CirclePrinter {
private:
    std::ostream & M_os;
public:
    CirclePrinter( std::ostream & os )
        : M_os( os )
      { }
    void operator()( const CircleT & circle )
      {
          M_os << " (circle "
               << ROUND( circle.circle_.center().x, 0.001 ) << ' '
               << ROUND( circle.circle_.center().y, 0.001 ) << ' '
               << ROUND( circle.circle_.radius(), 0.001 );
          if ( circle.color_.empty() )
          {
              M_os << " \"" << circle.color_ << '"';
          }
          M_os << ')';
      }
};

}

/*-------------------------------------------------------------------*/

struct CoachDebugClient::Impl {

    std::map< const CoachPlayerObject *, std::string > M_comment_map;

    std::vector< LineT > M_lines; //!< draw info: lines
    std::vector< TriangleT > M_triangles; //!< draw info: triangles
    std::vector< RectangleT > M_rectangles; //!< draw info: rectangles
    std::vector< CircleT > M_circles; //!< circles

};

/*-------------------------------------------------------------------*/
/*!

 */
CoachDebugClient::CoachDebugClient()
    : M_impl( new Impl() ),
      M_on( false ),
      M_connected( false ),
      M_write_mode( false ),
      M_main_buffer( "" ),
      M_target_unum( Unum_Unknown ),
      M_target_point( Vector2D::INVALIDATED ),
      M_message( "" )
{
    M_main_buffer.reserve( 8192 );
    M_message.reserve( 8192 );


    M_impl->M_lines.reserve( MAX_LINE );
    M_impl->M_triangles.reserve( MAX_TRIANGLE );
    M_impl->M_rectangles.reserve( MAX_RECT );
    M_impl->M_circles.reserve( MAX_CIRCLE );
}

/*-------------------------------------------------------------------*/
/*!

 */
CoachDebugClient::~CoachDebugClient()
{
    this->close();

    delete M_impl;
    M_impl = nullptr;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CoachDebugClient::connect( const std::string & hostname,
                           const int port )
{
    M_socket = std::shared_ptr< UDPSocket >( new UDPSocket( hostname.c_str(), port ) );

    if ( ! M_socket
         || M_socket->fd() == -1 )
    {
        std::cout << __FILE__ << ' ' << __LINE__
                  << ":cannot connect to the debug server host=["
                  << hostname << "] port=[" << port << "]"
                  << std::endl;
        if ( M_socket )
        {
            M_socket.reset();
        }

        M_connected = false;
        return false;
    }

    M_on = true;
    M_connected = true;
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CoachDebugClient::open( const std::string & log_dir,
                        const std::string & teamname )
{
    if ( M_server_log.is_open() )
    {
        M_server_log.close();
    }

    std::ostringstream filepath;
    if ( ! log_dir.empty() )
    {
        filepath << log_dir;
        if ( *log_dir.rbegin() != '/' )
        {
            filepath << '/';
        }
    }
    filepath << teamname << "-coach" << ".dcl";

    M_server_log.open( filepath.str().c_str() );

    if ( ! M_server_log.is_open() )
    {
        return false;
    }

    M_on = true;
    M_write_mode = true;
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachDebugClient::writeAll( const CoachWorldModel & world )
{
    if ( M_on )
    {
        this->buildString( world );

        if ( M_connected )
        {
            this->send();
        }

        if ( M_write_mode
             && world.gameMode().type() != GameMode::BeforeKickOff
             && world.gameMode().type() != GameMode::TimeOver )
        {
            this->write( world.time().cycle() );
        }

        this->clear();
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachDebugClient::close()
{
    if ( M_connected
         && M_socket )
    {
        M_socket->close();
        M_socket.reset();
    }

    if ( M_server_log.is_open() )
    {
        M_server_log.flush();
        M_server_log.close();
    }

    M_write_mode = false;
    M_on = false;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachDebugClient::buildString( const CoachWorldModel & world )
{
    std::ostringstream ostr;

    ostr << "((debug (format-version 5)) (time "
         << world.time().cycle() << ',';
    if ( world.gameMode().type() == GameMode::BeforeKickOff )
    {
        ostr << "0)";
    }
    else
    {
        ostr << world.time().stopped() << ')';
    }


    // coach identifier
    if ( world.ourSide() == LEFT )
    {
        ostr << " (s l c)";
    }
    else
    {
        ostr << " (s r c)";
    }

    // ball
    /*
      BALL_INFO ::=  (b POS_X POS_Y [VEL_X VEL_Y] [(c "COMMENT")])
    */
    const CoachBallObject & ball = world.ball();
    ostr << " (b "
         << ROUND( ball.pos().x, 0.01 ) << ' '
         << ROUND( ball.pos().y, 0.01 ) << ' '
         << ' ' << ROUND( ball.vel().x, 0.01 )
         << ' ' << ROUND( ball.vel().y, 0.01 )
         << ')';

    std::for_each( world.teammates().begin(),
                   world.teammates().end(),
                   PlayerPrinter( ostr, 't', M_impl->M_comment_map ) );

    std::for_each( world.opponents().begin(),
                   world.opponents().end(),
                   PlayerPrinter( ostr, 'o', M_impl->M_comment_map ) );

    // say message
    // if ( ! effector.getSayMessage().empty() )
    // {
    //     ostr << " (say \"";
    //     for ( std::vector< const SayMessage * >::const_iterator it = effector.sayMessageCont().begin();
    //           it != effector.sayMessageCont().end();
    //           ++it )
    //     {
    //         (*it)->printDebug( ostr );
    //     }
    //     ostr << " {" << effector.getSayMessage() << "}\")";
    // }

    // heard information
    // if ( world.audioMemory().time() == world.time() )
    // {
    //     ostr << " (hear ";
    //     world.audioMemory().printDebug( ostr );
    //     ostr << ')';
    // }

    // target number
    if ( M_target_unum != Unum_Unknown )
    {
        ostr << " (target-teammate " << M_target_unum << ")";
    }

    // target point
    if ( M_target_point.isValid() )
    {
        ostr << " (target-point "
             << M_target_point.x << " " << M_target_point.y
             << ")";
    }

    // message
    if ( ! M_message.empty() )
    {
        ostr << " (message \"" << M_message << "\")";
    }

    // lines
    std::for_each( M_impl->M_lines.begin(), M_impl->M_lines.end(),
                   LinePrinter( ostr ) );
    // triangles
    std::for_each( M_impl->M_triangles.begin(), M_impl->M_triangles.end(),
                   TrianglePrinter( ostr ) );
    // rectangles
    std::for_each( M_impl->M_rectangles.begin(), M_impl->M_rectangles.end(),
                   RectPrinter( ostr ) );
    // circles
    std::for_each( M_impl->M_circles.begin(), M_impl->M_circles.end(),
                   CirclePrinter( ostr ) );

    ostr << ")";

    M_main_buffer.assign( ostr.str() );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachDebugClient::send()
{
    if ( M_connected
         && M_socket )
    {
        if ( M_socket->writeDatagram( M_main_buffer.c_str(),
                                      M_main_buffer.length() + 1 ) == -1 )
        {
            std::cerr << "debug server send error" << std::endl;
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachDebugClient::write( const long & cycle )
{
#if 0
    if ( dlog.isOpen() )
    {
        //         char buf[32];
        //         snprintf( buf, 32, "%%%% step %ld\n", cycle );
        //         dlog.print( buf );
        //         dlog.print( "%% debug [" );
        //         dlog.print( M_main_buffer.c_str() );
        //         dlog.print( "]\n" );
        dlog.printDebugClientMessage( cycle, M_main_buffer );
    }
#endif

    if ( M_server_log.is_open() )
    {
        char buf[32];
        snprintf( buf, 32, "%%%% step %ld\n", cycle );

        M_server_log << buf << "%% debug [" << M_main_buffer << "]"
                     << std::endl;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachDebugClient::clear()
{
    M_main_buffer.erase();

    M_target_unum = Unum_Unknown;
    M_target_point.invalidate();
    M_message.erase();

    M_impl->M_comment_map.clear();

    M_impl->M_lines.clear();
    M_impl->M_triangles.clear();
    M_impl->M_rectangles.clear();
    M_impl->M_circles.clear();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachDebugClient::addMessage( const char * msg,
                              ... )
{
    if ( M_on )
    {
        va_list argp;
        va_start( argp, msg );
        vsnprintf( g_buffer, G_BUFFER_SIZE, msg, argp );
        va_end( argp );

        M_message += g_buffer;
        M_message += '|';
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachDebugClient::addMessage( const std::string & msg )
{
    if ( M_on )
    {
        M_message += msg;
        M_message += '|';
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachDebugClient::addComment( const CoachPlayerObject * player,
                              const char * msg,
                              ... )
{
    if ( M_on )
    {
        va_list argp;
        va_start( argp, msg );
        vsnprintf( g_buffer, G_BUFFER_SIZE, msg, argp );
        va_end( argp );

        M_impl->M_comment_map[player] += g_buffer;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachDebugClient::addLine( const Vector2D & from,
                           const Vector2D & to,
                           const char * color )
{
    if ( M_on )
    {
        if ( M_impl->M_lines.size() < MAX_LINE )
        {
            M_impl->M_lines.emplace_back( Segment2D( from, to ), color );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachDebugClient::addTriangle( const Triangle2D & tri,
                               const char * color )
{
    if ( M_on )
    {
        if ( M_impl->M_triangles.size() < MAX_TRIANGLE )
        {
            M_impl->M_triangles.emplace_back( tri, color );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachDebugClient::addRectangle( const Rect2D & rect,
                                const char * color )
{
    if ( M_on )
    {
        if ( M_impl->M_rectangles.size() < MAX_RECT )
        {
            M_impl->M_rectangles.emplace_back( rect, color );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachDebugClient::addCircle( const Circle2D & circle,
                             const char * color )
{
    if ( M_on )
    {
        if ( M_impl->M_circles.size() < MAX_CIRCLE )
        {
            M_impl->M_circles.emplace_back( circle, color );
        }
    }
}

}
