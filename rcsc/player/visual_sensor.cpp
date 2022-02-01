// -*-C++-*-

/*!
  \file visual_sensor.cpp
  \brief player's visual sensor Source File
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

#include "visual_sensor.h"

#include <rcsc/common/logger.h>

#include <iterator>
#include <algorithm>
#include <limits> // std::numeric_limits
#include <cstdlib>
#include <cstring>
#include <cmath> // HUGE_VAL

namespace rcsc {

/*-------------------------------------------------------------------*/

/*!
  \struct SeenDistCmp
  \brief functor to compare seen distance of observed object
*/
struct SeenDistCmp {
    /*!
      \brief operation function
      \param lhs left hand side variable
      \param rhs right hand side variable
      \return compared result
     */
    bool operator()( const VisualSensor::PolarT & lhs,
                     const VisualSensor::PolarT & rhs ) const
      {
          return lhs.dist_ < rhs.dist_;
      }
};


/*-------------------------------------------------------------------*/
/*!
  \brief stream operator
  \param os reference to output stream
  \param data printed data
  \return reference to output stream
*/
inline
std::ostream &
operator<<( std::ostream & os,
            const VisualSensor::BallT & data )
{
    os << "Ball--"
       << " dist=" << data.dist_
       << " dir=" << data.dir_
       << " dist_chng=" << data.dist_chng_
       << " dir_chng=" << data.dir_chng_;

    return os;
}

/*-------------------------------------------------------------------*/
/*!
  \brief stream operator
  \param os reference to output stream
  \param data printed data
  \return reference to output stream
*/
inline
std::ostream &
operator<<( std::ostream & os,
            const VisualSensor::PlayerT & data )
{
    os << "Player-- "
       << data.unum_
       << " dist=" << data.dist_
       << " dir=" << data.dir_
       << " dist_chng=" << data.dist_chng_
       << " dir_chng=" << data.dir_chng_
       << " body=" << data.body_
       << " neck=" << data.face_
       << " point_dir=" << data.arm_
       << " kicked=" << data.kicking_
       << " tackle=" << data.tackle_;

    return os;
}

/*-------------------------------------------------------------------*/
/*!
  \brief stream operator
  \param os reference to output stream
  \param data printed data
  \return reference to output stream
*/
inline
std::ostream &
operator<<( std::ostream & os,
            const VisualSensor::MarkerT & data )
{
    os << "Marker--"
       << " type=" << data.object_type_
       << " id=" << data.id_
       << " dist=" << data.dist_
       << " dir=" << data.dir_;

    return os;
}

/*-------------------------------------------------------------------*/
/*!
  \brief stream operator
  \param os reference to output stream
  \param data printed data
  \return reference to output stream
*/
inline
std::ostream &
operator<<( std::ostream & os,
            const VisualSensor::LineT & data )
{
    os << "Line--"
       << " id=" << data.id_
       << " dist=" << data.dist_
       << " dir=" << data.dir_;

    return os;
}

/*-------------------------------------------------------------------*/

const double VisualSensor::DIST_ERR = std::numeric_limits< double >::max();
const double VisualSensor::DIR_ERR = -360;

/*-------------------------------------------------------------------*/
/*!

*/
VisualSensor::VisualSensor()
    : M_time( -1, 0 ),
      M_their_team_name( "" )
{
    using namespace rcsc;
    typedef std::pair< std::string, MarkerID > MarkerPair;

    M_marker_map.insert( MarkerPair( "g l", Goal_L ) );
    M_marker_map.insert( MarkerPair( "g r", Goal_R ) );

    M_marker_map.insert( MarkerPair( "f c", Flag_C ) );
    M_marker_map.insert( MarkerPair( "f c t", Flag_CT ) );
    M_marker_map.insert( MarkerPair( "f c b", Flag_CB ) );
    M_marker_map.insert( MarkerPair( "f l t", Flag_LT ) );
    M_marker_map.insert( MarkerPair( "f l b", Flag_LB ) );
    M_marker_map.insert( MarkerPair( "f r t", Flag_RT ) );
    M_marker_map.insert( MarkerPair( "f r b", Flag_RB ) );

    M_marker_map.insert( MarkerPair( "f p l t", Flag_PLT ) );
    M_marker_map.insert( MarkerPair( "f p l c", Flag_PLC ) );
    M_marker_map.insert( MarkerPair( "f p l b", Flag_PLB ) );
    M_marker_map.insert( MarkerPair( "f p r t", Flag_PRT ) );
    M_marker_map.insert( MarkerPair( "f p r c", Flag_PRC ) );
    M_marker_map.insert( MarkerPair( "f p r b", Flag_PRB ) );

    M_marker_map.insert( MarkerPair( "f g l t", Flag_GLT ) );
    M_marker_map.insert( MarkerPair( "f g l b", Flag_GLB ) );
    M_marker_map.insert( MarkerPair( "f g r t", Flag_GRT ) );
    M_marker_map.insert( MarkerPair( "f g r b", Flag_GRB ) );

    M_marker_map.insert( MarkerPair( "f t l 50", Flag_TL50 ) );
    M_marker_map.insert( MarkerPair( "f t l 40", Flag_TL40 ) );
    M_marker_map.insert( MarkerPair( "f t l 30", Flag_TL30 ) );
    M_marker_map.insert( MarkerPair( "f t l 20", Flag_TL20 ) );
    M_marker_map.insert( MarkerPair( "f t l 10", Flag_TL10 ) );
    M_marker_map.insert( MarkerPair( "f t 0", Flag_T0 ) );
    M_marker_map.insert( MarkerPair( "f t r 10", Flag_TR10 ) );
    M_marker_map.insert( MarkerPair( "f t r 20", Flag_TR20 ) );
    M_marker_map.insert( MarkerPair( "f t r 30", Flag_TR30 ) );
    M_marker_map.insert( MarkerPair( "f t r 40", Flag_TR40 ) );
    M_marker_map.insert( MarkerPair( "f t r 50", Flag_TR50 ) );

    M_marker_map.insert( MarkerPair( "f b l 50", Flag_BL50 ) );
    M_marker_map.insert( MarkerPair( "f b l 40", Flag_BL40 ) );
    M_marker_map.insert( MarkerPair( "f b l 30", Flag_BL30 ) );
    M_marker_map.insert( MarkerPair( "f b l 20", Flag_BL20 ) );
    M_marker_map.insert( MarkerPair( "f b l 10", Flag_BL10 ) );
    M_marker_map.insert( MarkerPair( "f b 0", Flag_B0 ) );
    M_marker_map.insert( MarkerPair( "f b r 10", Flag_BR10 ) );
    M_marker_map.insert( MarkerPair( "f b r 20", Flag_BR20 ) );
    M_marker_map.insert( MarkerPair( "f b r 30", Flag_BR30 ) );
    M_marker_map.insert( MarkerPair( "f b r 40", Flag_BR40 ) );
    M_marker_map.insert( MarkerPair( "f b r 50", Flag_BR50 ) );

    M_marker_map.insert( MarkerPair( "f l t 30", Flag_LT30 ) );
    M_marker_map.insert( MarkerPair( "f l t 20", Flag_LT20 ) );
    M_marker_map.insert( MarkerPair( "f l t 10", Flag_LT10 ) );
    M_marker_map.insert( MarkerPair( "f l 0", Flag_L0 ) );
    M_marker_map.insert( MarkerPair( "f l b 10", Flag_LB10 ) );
    M_marker_map.insert( MarkerPair( "f l b 20", Flag_LB20 ) );
    M_marker_map.insert( MarkerPair( "f l b 30", Flag_LB30 ) );

    M_marker_map.insert( MarkerPair( "f r t 30", Flag_RT30 ) );
    M_marker_map.insert( MarkerPair( "f r t 20", Flag_RT20 ) );
    M_marker_map.insert( MarkerPair( "f r t 10", Flag_RT10 ) );
    M_marker_map.insert( MarkerPair( "f r 0", Flag_R0 ) );
    M_marker_map.insert( MarkerPair( "f r b 10", Flag_RB10 ) );
    M_marker_map.insert( MarkerPair( "f r b 20", Flag_RB20 ) );
    M_marker_map.insert( MarkerPair( "f r b 30", Flag_RB30 ) );

    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////

    M_marker_map_old.insert( MarkerPair( "goal l", Goal_L ) );
    M_marker_map_old.insert( MarkerPair( "goal r", Goal_R ) );

    M_marker_map_old.insert( MarkerPair( "flag c", Flag_C ) );
    M_marker_map_old.insert( MarkerPair( "flag c t", Flag_CT ) );
    M_marker_map_old.insert( MarkerPair( "flag c b", Flag_CB ) );
    M_marker_map_old.insert( MarkerPair( "flag l t", Flag_LT ) );
    M_marker_map_old.insert( MarkerPair( "flag l b", Flag_LB ) );
    M_marker_map_old.insert( MarkerPair( "flag r t", Flag_RT ) );
    M_marker_map_old.insert( MarkerPair( "flag r b", Flag_RB ) );

    M_marker_map_old.insert( MarkerPair( "flag p l t", Flag_PLT ) );
    M_marker_map_old.insert( MarkerPair( "flag p l c", Flag_PLC ) );
    M_marker_map_old.insert( MarkerPair( "flag p l b", Flag_PLB ) );
    M_marker_map_old.insert( MarkerPair( "flag p r t", Flag_PRT ) );
    M_marker_map_old.insert( MarkerPair( "flag p r c", Flag_PRC ) );
    M_marker_map_old.insert( MarkerPair( "flag p r b", Flag_PRB ) );

    M_marker_map_old.insert( MarkerPair( "flag g l t", Flag_GLT ) );
    M_marker_map_old.insert( MarkerPair( "flag g l b", Flag_GLB ) );
    M_marker_map_old.insert( MarkerPair( "flag g r t", Flag_GRT ) );
    M_marker_map_old.insert( MarkerPair( "flag g r b", Flag_GRB ) );

    M_marker_map_old.insert( MarkerPair( "flag t l 50", Flag_TL50 ) );
    M_marker_map_old.insert( MarkerPair( "flag t l 40", Flag_TL40 ) );
    M_marker_map_old.insert( MarkerPair( "flag t l 30", Flag_TL30 ) );
    M_marker_map_old.insert( MarkerPair( "flag t l 20", Flag_TL20 ) );
    M_marker_map_old.insert( MarkerPair( "flag t l 10", Flag_TL10 ) );
    M_marker_map_old.insert( MarkerPair( "flag t 0", Flag_T0 ) );
    M_marker_map_old.insert( MarkerPair( "flag t r 10", Flag_TR10 ) );
    M_marker_map_old.insert( MarkerPair( "flag t r 20", Flag_TR20 ) );
    M_marker_map_old.insert( MarkerPair( "flag t r 30", Flag_TR30 ) );
    M_marker_map_old.insert( MarkerPair( "flag t r 40", Flag_TR40 ) );
    M_marker_map_old.insert( MarkerPair( "flag t r 50", Flag_TR50 ) );

    M_marker_map_old.insert( MarkerPair( "flag b l 50", Flag_BL50 ) );
    M_marker_map_old.insert( MarkerPair( "flag b l 40", Flag_BL40 ) );
    M_marker_map_old.insert( MarkerPair( "flag b l 30", Flag_BL30 ) );
    M_marker_map_old.insert( MarkerPair( "flag b l 20", Flag_BL20 ) );
    M_marker_map_old.insert( MarkerPair( "flag b l 10", Flag_BL10 ) );
    M_marker_map_old.insert( MarkerPair( "flag b 0", Flag_B0 ) );
    M_marker_map_old.insert( MarkerPair( "flag b r 10", Flag_BR10 ) );
    M_marker_map_old.insert( MarkerPair( "flag b r 20", Flag_BR20 ) );
    M_marker_map_old.insert( MarkerPair( "flag b r 30", Flag_BR30 ) );
    M_marker_map_old.insert( MarkerPair( "flag b r 40", Flag_BR40 ) );
    M_marker_map_old.insert( MarkerPair( "flag b r 50", Flag_BR50 ) );

    M_marker_map_old.insert( MarkerPair( "flag l t 30", Flag_LT30 ) );
    M_marker_map_old.insert( MarkerPair( "flag l t 20", Flag_LT20 ) );
    M_marker_map_old.insert( MarkerPair( "flag l t 10", Flag_LT10 ) );
    M_marker_map_old.insert( MarkerPair( "flag l 0", Flag_L0 ) );
    M_marker_map_old.insert( MarkerPair( "flag l b 10", Flag_LB10 ) );
    M_marker_map_old.insert( MarkerPair( "flag l b 20", Flag_LB20 ) );
    M_marker_map_old.insert( MarkerPair( "flag l b 30", Flag_LB30 ) );

    M_marker_map_old.insert( MarkerPair( "flag r t 30", Flag_RT30 ) );
    M_marker_map_old.insert( MarkerPair( "flag r t 20", Flag_RT20 ) );
    M_marker_map_old.insert( MarkerPair( "flag r t 10", Flag_RT10 ) );
    M_marker_map_old.insert( MarkerPair( "flag r 0", Flag_R0 ) );
    M_marker_map_old.insert( MarkerPair( "flag r b 10", Flag_RB10 ) );
    M_marker_map_old.insert( MarkerPair( "flag r b 20", Flag_RB20 ) );
    M_marker_map_old.insert( MarkerPair( "flag r b 30", Flag_RB30 ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
VisualSensor::parse( const char * msg,
                     const std::string & team_name,
                     const double & version,
                     const GameTime & current )
{
    // never parse in same cycle
    if ( M_time == current )
    {
        return;
    }
    M_time = current;

    // clear old data
    clearAll();

    ObjectType object_type;

    MarkerT seen_marker;
    LineT seen_line;
    BallT seen_ball;
    PlayerT seen_player;

    seen_marker.reset();
    seen_line.reset();
    seen_ball.reset();
    seen_player.reset();

    // skip "(see "
    while ( *msg != ' ' ) ++msg;

    // skip "TIME"
    // it is necessary to check last paren ')',
    // because there are no information if player dose not see any object.
    while ( *msg != '(' && *msg != ')' ) ++msg;

    while ( *msg != '\0' && *msg != ')' )
    {
        // now msg must point the first of object info lisp token
        if ( *msg != '(' )
        {
            break;
        }

        ////////////////////////////////////////
        // identify object type
        object_type = getObjectTypeOf( *( msg + 2 ) );

        ////////////////////////////////////////
        // get object info
        // marker
        if ( object_type == Obj_Marker
             || object_type == Obj_Goal )
        {
            seen_marker.object_type_ = object_type;
            if ( parseMarker( msg, version, &seen_marker ) )
            {
                M_markers.push_back( seen_marker );
            }
            seen_marker.reset();
        }
        // behind marker
        else if ( object_type == Obj_Marker_Behind
                  || object_type == Obj_Goal_Behind )
        {
            seen_marker.object_type_ = object_type;
            if ( parseMarker( msg, version, &seen_marker ) )
            {
                M_behind_markers.push_back( seen_marker );
            }
            seen_marker.reset();
        }
        // player
        else if ( object_type == Obj_Player )
        {
            switch ( parsePlayer( msg, team_name, &seen_player ) ) {
            case Player_Teammate:
                M_teammates.push_back( seen_player );
                break;
            case Player_Unknown_Teammate:
                M_unknown_teammates.push_back( seen_player );
                break;
            case Player_Opponent:
                M_opponents.push_back( seen_player );
                break;
            case Player_Unknown_Opponent:
                M_unknown_opponents.push_back( seen_player );
                break;
            case Player_Unknown:
                M_unknown_players.push_back( seen_player );
                break;
            case Player_Low_Mode:
                break;
            default:
                std::cerr << "Illegal player object" << std::endl;
                break;
            }
            // reset value
            seen_player.reset();
        }
        // line
        else if ( object_type == Obj_Line )
        {
            if ( parseLine( msg, version, &seen_line ) )
            {
                M_lines.push_back( seen_line );
            }
            seen_line.reset();
        }
        // ball
        else if ( object_type == Obj_Ball )
        {
            if ( parseBall( msg, &seen_ball ) )
            {
                M_balls.push_back( seen_ball );
            }
            seen_ball.reset();
        }
        else // if ( object_type == Obj_Unknown )
        {
            std::cerr << "Unknown Object Type [" << *( msg + 2 ) << "]"
                      << std::endl;
        }

        // skip object info token
        while ( *msg != '\0' && *msg != ')' ) ++msg; // skip object info
        while ( *msg != '\0' && *msg != '(' ) ++msg; // skip to next object token
    } // main loop


    // sort by distance
    M_teammates.sort( SeenDistCmp() );
    M_unknown_teammates.sort( SeenDistCmp() );
    M_opponents.sort( SeenDistCmp() );
    M_unknown_opponents.sort( SeenDistCmp() );
    M_unknown_players.sort( SeenDistCmp() );

    M_markers.sort( SeenDistCmp() );
    M_behind_markers.sort( SeenDistCmp() );

    // line sort is very important !!
    M_lines.sort( SeenDistCmp() );

#if 0
    dlog.addText( Logger::SENSOR,
                  __FILE__" (parse) t=%d ut=%d o=%d uo=%d uk=%d"
                  "  seen markers=%d.  behind_markers=%d. lines=%d"
                  " ball=%d",
                  M_teammates.size(), M_unknown_teammates.size(),
                  M_opponents.size(), M_unknown_opponents.size(),
                  M_unknown_players.size()
                  M_markers.size(), M_behind_markers.size(),
                  M_lines.size(), M_balls.size() );
#endif
#if 0
    if ( M_markers.size() > 10 )
    {
        dlog.addText( Logger::SENSOR,
                      __FILE__" (parse) marker resize to 10 from %d",
                      M_markers.size() );
        M_markers.resize( 10 );
    }
#endif
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
VisualSensor::parseMarker( const char * tok,
                           const double version,
                           MarkerT * info )
{
    // get marker id
    if ( info->object_type_ == Obj_Marker_Behind
         || info->object_type_ == Obj_Goal_Behind )
    {
        info->id_ = Marker_Unknown;
    }
    else
    {
        // skip to first of object name
        while ( *tok == '(' ) ++tok; // skip to first identifier

        // get marker name
        int i = 0;
        std::string marker_name;
        while ( *( tok + i ) != ')' )
        {
            marker_name += *( tok + i );
            ++i;
        }

        // search marker id
        info->id_ = Marker_Unknown;
        if ( version >= 6.0 )
        {
            MarkerMap::const_iterator it = M_marker_map.find( marker_name );
            if ( it != M_marker_map.end() )
            {
                info->id_ = it->second;
            }
        }
        else
        {
            MarkerMap::const_iterator it = M_marker_map_old.find( marker_name );
            if ( it != M_marker_map_old.end() )
            {
                info->id_ = it->second;
            }
        }

        if ( info->id_ == Marker_Unknown )
        {
            std::cerr << "(VisualSensor::parseMarker) unknown marker "
                      << std::string( tok, 16 ) << "]"
                      << std::endl;
            return false;
        }
    }

    // skip object name
    while ( *tok != ')' ) ++tok; // skip all object name
    tok += 2; // skip paren & space

    char *next;

    // read dist
    info->dist_ = std::strtod( tok, &next );
    if ( info->dist_ == -HUGE_VAL
         || info->dist_ == HUGE_VAL )
    {
        std::cerr << "VisualSensor::parseMarker. distance read error.["
                  << std::string( tok, 16 ) << "]"
                  << std::endl;
        return false;
    }
    tok = next;

    // check view quality
    if ( *tok == ')' )
    {
        //std::cerr << "VisualSensor:: parseMarker: view quality is LOW ??\n";
        return false;
    }

    // read dir
    info->dir_ = std::strtod( tok, NULL );
    if ( info->dir_ == -HUGE_VAL
         || info->dir_ == HUGE_VAL )
    {
        std::cerr << "VisualSensor::parseMarker: dir read error.["
                  << std::string( tok, 16 ) << "]"
                  << std::endl;
        return false;
    }

    // chng data is not parsed.
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
VisualSensor::parseLine( const char * tok,
                         const double & version,
                         LineT * info )
{
#if 0
    // ((l <side>) <dist> <dir>))
    // ((L <side>) <dist> <dir>))
    // ((line <side>) <dist> <dir>))
    // ((Line <side>) <dist> <dir>))

    char id[8];
    char side;
    int n_read = 0;
    if ( std::sscanf( tok, " ( ( %[^ ] %c ) %lf %lf ) %n",
                      id, &side,
                      &(info->dist_),
                      &(info->dir_),
                      &n_read  ) == 4 )
    {
        // high quality line
    }
    else if ( std::sscanf( tok, " ( ( %[^ ] %c) %lf ) %n",
                           id, &side,
                           &(info->dir_),
                           &n_read ) == 3 )
    {
        // low quality line
    }
    else
    {
        std::cerr << "Unsupported line message "
                  << std::string( tok, 16 ) << std::endl;
        info->id_ = Line_Unknown;
        return false;
    }

    switch ( side ) {
    case 'l':
        info->id_ = Line_Left;
        break;
    case 'r':
        info->id_ = Line_Right;
        break;
    case 't':
        info->id_ = Line_Top;
        break;
    case 'b':
        info->id_ = Line_Bottom;
        break;
    default:
        std::cerr << "Unknown line type [" << *( tok + 2 ) << "]"
                  << std::endl;
        info->id_ = Line_Unknown;
        return false;
    }

    return true;

#else
    // skip to first of object name
    while ( *tok == '(' ) ++tok;

    // check line name
    int i = ( version >= 6.0 ? 2 : 5 );

    switch ( *(tok + i) ) {
    case 'l':
        info->id_ = Line_Left;
        break;
    case 'r':
        info->id_ = Line_Right;
        break;
    case 't':
        info->id_ = Line_Top;
        break;
    case 'b':
        info->id_ = Line_Bottom;
        break;
    default:
        std::cerr << "Unknown line type [" << *( tok + 2 ) << "]"
                  << std::endl;
        info->id_ = Line_Unknown;
        return false;
    }

    // skip object name
    while ( *tok != ')' ) ++tok; // skip all object name
    tok += 2; // skip paren & space

    char *next;

    // read dist
    info->dist_ = std::strtod( tok, &next );
    if ( info->dist_ == -HUGE_VAL
         || info->dist_ == HUGE_VAL )
    {
        std::cerr << "VisualSensor:: parseLine: distance read error.["
                  << std::string( tok, 16 ) << "]"
                  << std::endl;
        return false;
    }
    tok = next;

    // check view quality
    if ( *tok == ')' )
    {
        //std::cerr << "VisualSensor:: parseLine: view quality is LOW ??\n";
        return false;
    }

    // read dir
    info->dir_ = std::strtod( tok, NULL );
    if ( info->dir_ == -HUGE_VAL
         || info->dir_== HUGE_VAL )
    {
        std::cerr << "VisualSensor::parseLine: dirread error.["
                  << std::string( tok, 16 ) << "]"
                  << std::endl;;
        return false;
    }

    return true;
#endif
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
VisualSensor::parseBall( const char * tok,
                         BallT * info )
{
    // skip all object name
    while ( *tok != ')' ) ++tok;
    tok += 2; // skip space & paren

    char *next;

    // read dist
    info->dist_ = std::strtod( tok, &next );
    if ( info->dist_ == -HUGE_VAL
         || info->dist_ == HUGE_VAL )
    {
        std::cerr << "VisualSensor::parseBall: distance read error.["
                  << std::string( tok, 16 ) << "]"
                  << std::endl;
        return false;
    }
    tok = next;

    // check view quality
    if ( *tok == ')' )
    {
        //std::cerr << "VisualSensor:: parseBall: view quality is LOW ??\n";
        return false;
    }

    // read dir
    info->dir_ = std::strtod( tok, &next );
    if ( info->dir_ == -HUGE_VAL
         || info->dir_ == HUGE_VAL )
    {
        std::cerr << "VisualSensor::parseBall: dir read error. ["
                  << std::string( tok, 16 ) << "]"
                  << std::endl;
        return false;
    }
    tok = next;

    // read velocity info. order is dist_chg -> dir_chg
    if ( *tok != ')' )
    {
        info->dist_chng_ = std::strtod( tok, &next );
        tok = next;
        info->dir_chng_ = std::strtod( tok, NULL );
        info->has_vel_ = true;
        if ( info->dist_chng_ == -HUGE_VAL
             || info->dist_chng_ == HUGE_VAL
             || info->dir_chng_ == -HUGE_VAL
             || info->dir_chng_ == HUGE_VAL )
        {
            std::cerr << "VisualSensor:: parseBall. chng read error.["
                      << std::string( tok, 16 ) << "]"
                      << std::endl;
            info->dist_chng_ = 0.0;
            info->dir_chng_ = 0.0;
            info->has_vel_ = false;
            return false;
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
VisualSensor::PlayerInfoType
VisualSensor::parsePlayer( const char * tok,
                           const std::string & team_name,
                           PlayerT * info )
{
    PlayerInfoType result_type = Player_Illegal;

    // skip to first of object name
    while ( *tok == '(' ) ++tok;

    // count the space in object name for player identify
    // (p), (p "TEAMNAME"), (p "TEAMNAME" UNUM), (p "TEAMNAME" UNUM goalie)
    int n_space = 0;
    for ( int i = 1; *(tok + i) != ')' && *(tok + i) != '\0'; ++i )
    {
        if ( *(tok + i) == ' ' ) ++n_space;
    }

    // check player name
    // ((p "TEAMNAME" UNUM [goalie]) ...)

    // check teamname
    if ( n_space > 0 ) // exist team name
    {
        while ( *tok != '\"' ) ++tok; // " skip to team name
        ++tok; // skip '"'

        const size_t len = team_name.length();

        if ( *( tok + len ) == '\"' // "
             && team_name.compare( 0, len, tok, len ) == 0 )
        {
            result_type = Player_Unknown_Teammate;
        }
        else
        {
            result_type = Player_Unknown_Opponent;
            if ( M_their_team_name.empty() )
            {
                while ( *tok != '\"' ) M_their_team_name += *tok++; // "
                // std::cerr << "copy opponent team name : "
                // << M_opponent_team_name << std::endl;
            }
        }
    }
    else
    {
        result_type = Player_Unknown;
    }

    // check unum
    if ( n_space > 1 )
    {
        while ( *tok != ' ' ) ++tok;
        info->unum_ = std::atoi( tok );
        // we can get all player identifier
        result_type = ( result_type == Player_Unknown_Teammate
                        ? Player_Teammate
                        : Player_Opponent );
    }
    else
    {
        info->unum_ = Unum_Unknown;
    }

    // check goalie flag
    if ( n_space > 2 )
    {
        info->goalie_ = true;
    }

    // skip all player name
    while ( *tok != ')' ) ++tok;
    ++tok;

    // "tok" must point object name next space

    // check positional info pattern
    // " <DIST> <DIR> <DISTCH> <DIRCH> <BODY> <HEAD> <POINTDIR> <TACKLE|KICK>)" : space = 8
    // " <DIST> <DIR> <DISTCH> <DIRCH> <BODY> <HEAD> <POINTDIR>)" : space = 7
    // " <DIST> <DIR> <DISTCH> <DIRCH> <BODY> <HEAD> <TACKLE|KICK>)" : space = 7
    // " <DIST> <DIR> <DISTCH> <DIRCH> <BODY> <HEAD>)" : space = 6
    // " <DIST> <DIR> <DISTCH> <DIRCH> <BODY>)" : space = 5  only sserver-4
    // " <DIST> <DIR> <DISTCH> <DIRCH>)" : space = 4
    // " <DIST> <DIR> <POINTDIR> <TACKLE|KICK>)" : space = 4
    // " <DIST> <DIR> <POINTDIR>)" : space = 3
    // " <DIST> <DIR> <TACKLE|KICK>)" : space = 3
    // " <DIST> <DIR>)" : space = 2
    // " <DIR>)" : space = 1

    // count space in positional info
    n_space = 0;
    for ( int i = 0; *(tok + i) != ')' && *(tok + i) != '\0'; ++i )
    {
        if ( *( tok + i ) == ' ' ) ++n_space;
    }

    char *next;

    // read each value on each pattern

    // <DIST> <DIR> <DISTCH> <DIRCH> <BODY> <HEAD> <POINTDIR> <TACKLE>
    if ( n_space == 8 )
    {
        info->dist_ = std::strtod( tok, &next ); tok = next;
        info->dir_  = std::strtod( tok, &next ); tok = next;
        info->dist_chng_ = std::strtod( tok, &next ); tok = next;
        info->dir_chng_  = std::strtod( tok, &next ); tok = next;
        info->body_ = std::strtod( tok, &next ); tok = next;
        info->face_ = std::strtod( tok, &next ); tok = next;
        info->arm_ = std::strtod( tok, &next ); tok = next;
        info->has_vel_ = true;
        if ( *(tok + 1) == 'k' ) info->kicking_ = true;
        if ( *(tok + 1) == 't' ) info->tackle_ = true;
    }
    // <DIST> <DIR> <DISTCH> <DIRCH> <BODY> <HEAD> <POINTDIR>
    // <DIST> <DIR> <DISTCH> <DIRCH> <BODY> <HEAD> <TACKLE>
    else if ( n_space == 7 )
    {
        info->dist_ = std::strtod( tok, &next ); tok = next;
        info->dir_  = std::strtod( tok, &next ); tok = next;
        info->dist_chng_ = std::strtod( tok, &next ); tok = next;
        info->dir_chng_  = std::strtod( tok, &next ); tok = next;
        info->body_ = std::strtod( tok, &next ); tok = next;
        info->face_ = std::strtod( tok, &next ); tok = next;
        info->has_vel_ = true;
        if ( *(tok + 1) == 'k' )
        {
            info->kicking_ = true;
        }
        else if ( *(tok + 1) == 't' )
        {
            info->tackle_ = true;
        }
        else
        {
            info->arm_ = std::strtod( tok, NULL );
        }
    }
    // <DIST> <DIR> <DISTCH> <DIRCH> <BODY> <HEAD>
    else if ( n_space == 6 )
    {
        info->dist_ = std::strtod( tok, &next ); tok = next;
        info->dir_  = std::strtod( tok, &next ); tok = next;
        info->dist_chng_ = std::strtod( tok, &next ); tok = next;
        info->dir_chng_  = std::strtod( tok, &next ); tok = next;
        info->body_ = std::strtod( tok, &next ); tok = next;
        info->face_ = std::strtod( tok, NULL );
        info->has_vel_ = true;
    }
    // <DIST> <DIR> <DISTCH> <DIRCH> <BODY>
    else if ( n_space == 5 )
    {
        info->dist_ = std::strtod( tok, &next ); tok = next;
        info->dir_  = std::strtod( tok, &next ); tok = next;
        info->dist_chng_ = std::strtod( tok, &next ); tok = next;
        info->dir_chng_  = std::strtod( tok, &next ); tok = next;
        info->body_ = std::strtod( tok, NULL );
        info->face_ = 0.0;
        info->has_vel_ = true;
    }
    // <DIST> <DIR> <DISTCH> <DIRCH>
    // <DIST> <DIR> <POINTDIR> <TACKLE>
    else if ( n_space == 4 )
    {
        info->dist_ = std::strtod( tok, &next ); tok = next;
        info->dir_  = std::strtod( tok, &next ); tok = next;
        double tmp = std::strtod( tok, &next ); tok = next;
        if ( *(tok + 1) == 'k' )
        {
            info->arm_ = tmp;
            info->kicking_ = true;
        }
        else if ( *(tok + 1) == 't' )
        {
            info->arm_ = tmp;
            info->tackle_ = true;
        }
        else
        {
            info->dist_chng_ = tmp;
            info->dir_chng_ = std::strtod( tok, NULL );
        }
    }
    // <DIST> <DIR> <POINTDIR>
    // <DIST> <DIR> <TACKLE>
    else if ( n_space == 3 )
    {
        info->dist_ = std::strtod( tok, &next ); tok = next;
        info->dir_  = std::strtod( tok, &next ); tok = next;
        if ( *(tok + 1) == 'k' )
        {
            info->kicking_ = true;
        }
        else if ( *(tok + 1) == 't' )
        {
            info->tackle_ = true;
        }
        else
        {
            info->arm_ = std::strtod( tok, NULL );
        }
    }
    // <DIST> <DIR>
    else if ( n_space == 2 )
    {
        info->dist_ = std::strtod( tok, &next ); tok = next;
        info->dir_  = std::strtod( tok, NULL );
    }
    else
    {
        //std::cerr << "ViewQuality is Low ?? Unexpected player see info pattern\n   ["
        //          << tok << "]" << std::endl;
        return Player_Low_Mode;
    }

    // check error
    if ( info->dist_ < 0.0
         || info->dist_ == -HUGE_VAL
         || info->dist_ == HUGE_VAL
         || info->dir_ == -HUGE_VAL
         || info->dir_ == HUGE_VAL )
    {
        std::cerr << "VisualSensor::parsePlayer. polar value error."
                  << " dist=" << info->dist_ << " dir=" << info->dir_
                  << std::endl;
        return Player_Illegal;
    }

    if ( info->has_vel_
         && ( info->dist_chng_ == -HUGE_VAL
              || info->dist_chng_ == HUGE_VAL
              || info->dir_chng_ == -HUGE_VAL
              || info->dir_chng_ == HUGE_VAL )
         )
    {
        std::cerr << "VisualSensor::parsePlayer. chng value error"
                  << std::endl;
        info->dist_chng_
            = info->dir_chng_
            = 0.0;
        info->has_vel_ = false;
    }

    if ( info->body_ == -HUGE_VAL
         || info->body_ == HUGE_VAL )
    {
        std::cerr << "VisualSensor::parsePlayer. body value error"
                  << std::endl;
        info->body_ = DIR_ERR;
    }

    if ( info->face_ == -HUGE_VAL
         || info->face_ == HUGE_VAL )
    {
        std::cerr << "VisualSensor::parsePlayer. neck value error"
                  << std::endl;
        info->face_ = DIR_ERR;
    }

    if ( info->arm_ == -HUGE_VAL
         || info->arm_ == HUGE_VAL )
    {
        std::cerr << "VisualSensor::parsePlayer. point value error"
                  << std::endl;
        info->arm_ = DIR_ERR;
    }

    return result_type;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
VisualSensor::clearAll()
{
    M_balls.clear();

    M_markers.clear();
    M_behind_markers.clear();
    M_lines.clear();

    M_teammates.clear();
    M_unknown_teammates.clear();
    M_opponents.clear();
    M_unknown_opponents.clear();
    M_unknown_players.clear();
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
VisualSensor::print( std::ostream & os )
{
    //os<< "opp team: " << M_opponent_team_name << std::endl;

    os << "-----BallCount " << M_balls.size() << "-----\n";
    std::copy( M_balls.begin(), M_balls.end(),
               std::ostream_iterator< BallT >( os, "\n" ) );

    os << "-----TeammateCount " << M_teammates.size() << "-----\n";
    std::copy( M_teammates.begin(), M_teammates.end(),
               std::ostream_iterator< PlayerT >( os, "\n" ) );

    os << "-----UnknownTeammateCount " << M_unknown_teammates.size() << "-----\n";
    std::copy( M_unknown_teammates.begin(), M_unknown_teammates.end(),
               std::ostream_iterator< PlayerT >( os, "\n" ) );

    os << "-----OpponentCount " << M_opponents.size() << "-----\n";
    std::copy( M_opponents.begin(), M_opponents.end(),
               std::ostream_iterator< PlayerT >( os, "\n" ) );

    os << "-----UnknownOpponentCount " << M_unknown_opponents.size() << "-----\n";
    std::copy( M_unknown_opponents.begin(), M_unknown_opponents.end(),
               std::ostream_iterator< PlayerT >( os, "\n" ) );

    os << "-----UnknownPlayerCount " << M_unknown_players.size() << "-----\n";
    std::copy( M_unknown_players.begin(), M_unknown_players.end(),
               std::ostream_iterator< PlayerT >( os, "\n" ) );

    os << "-----MarkerCount " << M_markers.size() << "-----\n";
    std::copy( M_markers.begin(), M_markers.end(),
               std::ostream_iterator< MarkerT >( os, "\n" ) );

    os << "-----LineCount " << M_lines.size() << "-----\n";
    std::copy( M_lines.begin(), M_lines.end(),
               std::ostream_iterator< LineT >( os, "\n" ) );
    return os << std::endl;
}

}
