// -*-c++-*-

/*!
  \file debug_client.cpp
  \brief Interface for Soccer Viewer & soccerwindow2 Source File
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

#include "debug_client.h"

#include "action_effector.h"
#include "world_model.h"
#include "player_object.h"
#include "abstract_player_object.h"
#include "say_message_builder.h"

#include <rcsc/common/audio_memory.h>
#include <rcsc/net/udp_socket.h>

#include <algorithm>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cmath>

namespace rcsc {

// accessible only from this file
namespace {

//! max send buffer size for debug client
#define G_BUFFER_SIZE 8192*4

//! global variable
static char g_buffer[G_BUFFER_SIZE];

/*-------------------------------------------------------------------*/

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

class PlayerPtrPrinter {
private:
    std::ostream & M_os;
    const SideID M_our_side;
    const std::map< const PlayerObject *, std::string > & comment_map_;
public:
    PlayerPtrPrinter( std::ostream & os,
                      const SideID our_side,
                      const std::map< const PlayerObject *, std::string > & comment_map )
        : M_os( os ),
          M_our_side( our_side ),
          comment_map_( comment_map )
      { }

    void operator()( const PlayerObject * p )
      {
          M_os << " (";

          //
          // side, unum, player type
          //
          if ( p->side() == NEUTRAL )
          {
              M_os << 'u';
          }
          else if ( p->side() == M_our_side )
          {
              if ( p->unum() != Unum_Unknown )
              {
                  M_os << "t " << p->unum();
                  if ( p->playerTypePtr() )
                  {
                      M_os << ' ' << p->playerTypePtr()->id();
                  }
                  else
                  {
                      M_os << " -1";
                  }
              }
              else
              {
                  M_os << "ut";
              }
          }
          else
          {
              if ( p->unum() != Unum_Unknown )
              {
                  M_os << "o " << p->unum();
                  if ( p->playerTypePtr() )
                  {
                      M_os << ' ' << p->playerTypePtr()->id();
                  }
                  else
                  {
                      M_os << " -1";
                  }
              }
              else
              {
                  M_os << "uo";
              }
          }

          //
          // x, y
          //
          M_os << ' ' << ROUND( p->pos().x, 0.01 )
               << ' ' << ROUND( p->pos().y, 0.01 );

          //
          // body
          //
          if ( p->bodyValid() )
          {
              M_os << " (bd " << rint( p->body().degree() ) << ')';
          }

          //
          // pointto
          //
          if ( p->pointtoCount() < 10 )
          {
              M_os << " (pt " << rint( p->pointtoAngle().degree() ) << ')';
          }

          //
          // comment (goalie, unumCount, posPount, velCount, [vel], faceCount, [{t|k}], [y], ballReachStep, comment)
          //
          M_os << " (c \"";

          if  ( p->goalie() )
          {
              M_os << 'G';
          }

          if ( p->unum() != Unum_Unknown )
          {
              M_os << 'u' << p->unumCount();
          }

          M_os << 'p' << p->posCount()
               << 'v' << p->velCount();

          if ( p->velCount() <= 100 )
          {
              M_os << '(' << ROUND( p->vel().x, 0.001 )
                   << ' ' << ROUND( p->vel().y, 0.001 )
                   << ')';
          }
          M_os << 'f' << p->faceCount();

          if ( p->isTackling() )
          {
              M_os << "t";
          }
          else if ( p->isKicking() )
          {
              M_os << "k";
          }

          if ( p->card() == YELLOW )
          {
              M_os << "y";
          }

          M_os << ',' << p->ballReachStep();

          std::map< const PlayerObject *, std::string >::const_iterator it = comment_map_.find( p );
          if ( it != comment_map_.end() )
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

} // end of noname namespace

/*-------------------------------------------------------------------*/

struct DebugClient::Impl {

    std::string M_self_comment;
    std::map< const PlayerObject *, std::string > M_comment_map;

    std::vector< LineT > M_lines; //!< draw info: lines
    std::vector< TriangleT > M_triangles; //!< draw info: triangles
    std::vector< RectangleT > M_rectangles; //!< draw info: rectangles
    std::vector< CircleT > M_circles; //!< circles

};

/*-------------------------------------------------------------------*/
/*!

 */
DebugClient::DebugClient()
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
DebugClient::~DebugClient()
{
    this->close();

    delete M_impl;
    M_impl = nullptr;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
DebugClient::connect( const std::string & hostname,
                      const int port )
{
    M_socket = std::shared_ptr< UDPSocket >( new UDPSocket( hostname.c_str(), port ) );

    if ( ! M_socket
         || M_socket->fd() == -1 )
    {
        std::cout << "cannot connect to the debug server host=["
                  << hostname << "] port=[" << port << "]"
                  << std::endl;
        if ( M_socket ) M_socket.reset();
        M_connected = false;
        return;
    }
    M_on = true;
    M_connected = true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
DebugClient::open( const std::string & log_dir,
                   const std::string & teamname,
                   const int unum )
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
    filepath << teamname << '-' << unum << ".dcl";

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
DebugClient::writeAll( const WorldModel & world,
                       const ActionEffector & effector )
{
    if ( M_on )
    {
        this->buildString( world, effector );

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
DebugClient::close()
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
DebugClient::buildString( const WorldModel & world,
                          const ActionEffector & effector )
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

    // self
    /*
      SELF ::=  (s SIDE PLAYER_NUMBER POS_X POS_Y VEL_X VEL_Y
      BODY_DIRECTION FACE_DIRECTION  [(c "COMMENT")])
      where SIDE is 'l' or 'r'.
      where PLAYER_NUMBER is 1 to 11.
      where POS_X and POS_Y is absolute coordinate.
      where VEL_X and VEL_Y is absolute velocity.
      where BODY_DIRECTION is absolute player body direction (degree).
      where NECK_DIRECTION is body relative face direction (degree).
      This is equals to (absolute_face_direction - BODY_DIRECTION).
      where COMMENT is a string.
      Typically, it is used as information accuracy.
    */
    if ( world.self().posValid() )
    {
        ostr << " (s "
             << ( world.ourSide() == LEFT ? "l " : "r " )
             << world.self().unum() << ' '
             << world.self().playerType().id() << ' '
             << ROUND(world.self().pos().x, 0.01) << ' '
             << ROUND(world.self().pos().y, 0.01) << ' '
             << ROUND(world.self().vel().x, 0.01) << ' '
             << ROUND(world.self().vel().y, 0.01) << ' '
             << ROUND(world.self().body().degree(), 0.1) << ' '
             << ROUND(world.self().neck().degree(), 0.1)
             << " (c \"p" << world.self().posCount()
             << 'v' << world.self().velCount();
        if ( world.self().velValid() )
        {
            ostr << '(' << ROUND( world.self().vel().x, 0.001 )
                 << ' ' << ROUND( world.self().vel().y, 0.001 ) << ')';
        }
        ostr << 'f' << world.self().faceCount();
        if ( world.self().card() == YELLOW )
        {
            ostr << "y";
        }
        if ( ! M_impl->M_self_comment.empty() )
        {
            ostr << '|' << M_impl->M_self_comment;
        }
        ostr << "\"))";
    }

    // ball
    /*
      BALL_INFO ::=  (b POS_X POS_Y [VEL_X VEL_Y] [(c "COMMENT")])
    */
    if ( world.ball().posValid() )
    {
        ostr << " (b "
             << ROUND(world.ball().pos().x, 0.01) << ' '
             << ROUND(world.ball().pos().y, 0.01);
        if ( world.ball().velValid() )
        {
            ostr << ' ' << ROUND(world.ball().vel().x, 0.01)
                 << ' ' << ROUND(world.ball().vel().y, 0.01);
        }
        ostr << " (c \"g" << world.ball().posCount()
             << 'r' << world.ball().rposCount()
            //<< "(" << ROUND(world.ball().rpos().x, 0.01)
            // << ", " << ROUND(world.ball().rpos().y, 0.01) << ')'
             << 'v' << world.ball().velCount()
            //<< "(" << ROUND(world.ball().vel().x, 0.01)
            // << ", " << ROUND(world.ball().vel().y, 0.01) << ')'
             << "\"))";
    }

    // players
    /*
      PLAYER_INFO ::=  (TEAM [PLAYER_NUMBER] POS_X POS_Y
      [(bd BODY_DIRECTION)] [(c "COMMENT")])
      TEAM is one of follows.
      't' (teammate), 'o' (opponent),
      'u' (unknown), 'ut' (unknown teammate), 'ut' (unknown opponent).
      When TEAM is 't' or 'o', PLAYER_NUMBER must be specified.
      Otherwise PLAYER_NUMBER must not be specified.
      Body direction and comment is optional.
    */

    std::for_each( world.teammates().begin(),
                   world.teammates().end(),
                   PlayerPtrPrinter( ostr, world.ourSide(), M_impl->M_comment_map ) );
    std::for_each( world.opponents().begin(),
                   world.opponents().end(),
                   PlayerPtrPrinter( ostr, world.ourSide(), M_impl->M_comment_map ) );

    // say message
    if ( ! effector.getSayMessage().empty() )
    {
        ostr << " (say \"";
        for ( const SayMessage::Ptr & i : effector.sayMessageCont() )
        {
            i->printDebug( ostr );
        }
        ostr << " {" << effector.getSayMessage() << "}\")";
    }

    // heard information
    if ( world.audioMemory().time() == world.time() )
    {
        ostr << " (hear ";
        world.audioMemory().printDebug( ostr );
        ostr << ')';
    }

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
DebugClient::send()
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
DebugClient::write( const long & cycle )
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
DebugClient::clear()
{
    M_main_buffer.erase();

    M_target_unum = Unum_Unknown;
    M_target_point.invalidate();
    M_message.erase();

    M_impl->M_self_comment.clear();
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
DebugClient::addMessage( const char * msg,
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
DebugClient::addMessage( const std::string & msg )
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
DebugClient::addSelfComment( const char * msg,
                             ... )
{
    if ( M_on )
    {
        va_list argp;
        va_start( argp, msg );
        vsnprintf( g_buffer, G_BUFFER_SIZE, msg, argp );
        va_end( argp );

        M_impl->M_self_comment += g_buffer;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
DebugClient::addComment( const PlayerObject * player,
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
DebugClient::addLine( const Vector2D & from,
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
DebugClient::addTriangle( const Triangle2D & tri,
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
DebugClient::addRectangle( const Rect2D & rect,
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
DebugClient::addCircle( const Circle2D & circle,
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
