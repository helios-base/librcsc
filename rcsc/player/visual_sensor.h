// -*-c++-*-

/*!
  \file visual_sensor.h
  \brief player's visual sensor Header File
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

#ifndef RCSC_PLAYER_VISUAL_SENSOR_H
#define RCSC_PLAYER_VISUAL_SENSOR_H

#include <rcsc/game_time.h>
#include <rcsc/types.h>

#include <unordered_map>
#include <list>
#include <vector>
#include <string>
#include <iostream>

namespace rcsc {

/*!
  \class VisualSensor
  \brief player's parsed visual info holder
*/
class VisualSensor {
public:
    static const double DIST_ERR; //!< error value
    static const double DIR_ERR; //!< error value

    /*!
      \brief seen object type
    */
    enum ObjectType {
        Obj_Goal,
        Obj_Goal_Behind,
        Obj_Marker,
        Obj_Marker_Behind,
        Obj_Line,
        Obj_Ball,
        Obj_Player,
        //Obj_Player_Behind,
        Obj_Unknown
    };

    /*!
      \brief seen player identification type
    */
    enum PlayerInfoType {
        Player_Teammate = 10,
        Player_Unknown_Teammate = 11,
        Player_Opponent = 20,
        Player_Unknown_Opponent = 21,
        Player_Unknown = 30,
        Player_Low_Mode,
        Player_Illegal
    };

    /*!
      \brief observable object info
    */
    struct PolarT {
        double dist_; //!< seen distance
        double dir_; //!< seen dir

        /*!
          \brief init member variables by error value
        */
        PolarT()
            : dist_( VisualSensor::DIST_ERR )
            , dir_( VisualSensor::DIR_ERR )
          { }
        /*!
          \brief clear all data
        */
        void reset()
          {
              dist_ = VisualSensor::DIST_ERR;
              dir_ = VisualSensor::DIR_ERR;
          }
    };

    /*!
      \brief seen movable object info
    */
    struct MovableT
        : public PolarT {
        bool has_vel_; //!< true if velocity is seen
        double dist_chng_; //!< seen dist change
        double dir_chng_; //!< seen dir change

        /*!
          \brief init member variables by error value
        */
        MovableT()
            : PolarT()
            , has_vel_( false )
            , dist_chng_( 0.0 )
            , dir_chng_( 0.0 )
          { }
        /*!
          \brief clear all data
        */
        void reset()
          {
              PolarT::reset();
              has_vel_ = false;
              dist_chng_ = 0.0;
              dir_chng_ = 0.0;
          }
    };

    /*!
      \brief seen line info
    */
    struct LineT
        : public PolarT {
        //! line Id
        LineID id_;

        /*!
          \brief init member variables by error value
        */
        LineT()
            : PolarT()
            , id_( Line_Unknown )
          { }
        /*!
          \brief clear all data
        */
        void reset()
          {
              PolarT::reset();
              id_ = Line_Unknown;
          }
    };

    /*!
      \brief seen marker flag info
    */
    struct MarkerT
        : public PolarT {
        //! behind or not. for close marker estimation
        ObjectType object_type_;
        //! marker flag Id
        MarkerID id_;

        /*!
          \brief init member variables by error value
        */
        MarkerT()
            : PolarT()
            , object_type_( VisualSensor::Obj_Unknown )
            , id_( Marker_Unknown )
          { }
        /*!
          \brief clear all data
        */
        void reset()
          {
              PolarT::reset();
              object_type_ = VisualSensor::Obj_Unknown;
              id_ = Marker_Unknown;
          }
    };

    /*!
      \brief seen ball info
    */
    struct BallT
        : public MovableT {

        /*!
          \brief init member variables by error value
        */
        BallT()
            : MovableT()
          { }
        /*!
          \brief clear all data
        */
        void reset()
          {
              MovableT::reset();
          }
    };

    /*!
      \brief seen player info
    */
    struct PlayerT
        : public MovableT {
        int    unum_; //!< seen uniform number
        bool   goalie_; //!< true if goalie seen
        double body_; //!< seen body dir. (relative to self face)
        double face_; //!< seen face dir. (relative to self face)
        double arm_; //!< seen pointing dir. (relative to self face)
        bool   kicking_; //!< true if kicked
        bool   tackle_; //!< true if tackling

        /*!
          \brief init member variables by error value
        */
        PlayerT()
            : MovableT(),
              unum_( Unum_Unknown ),
              goalie_( false ),
              body_( VisualSensor::DIR_ERR ),
              face_( VisualSensor::DIR_ERR ),
              arm_( VisualSensor::DIR_ERR ),
              kicking_( false ),
              tackle_( false )
          { }
        /*!
          \brief clear all data
        */
        void reset()
          {
              MovableT::reset();
              unum_ = Unum_Unknown;
              goalie_ = false;
              body_ = VisualSensor::DIR_ERR;
              face_ = VisualSensor::DIR_ERR;
              arm_ = VisualSensor::DIR_ERR;
              kicking_ = false;
              tackle_ = false;
          }
    };

    typedef std::unordered_map< std::string, MarkerID > MarkerMap;

    typedef std::vector< BallT > BallCont; //!< observed ball container
    typedef std::list< MarkerT > MarkerCont; //!< observed marker container
    typedef std::list< LineT > LineCont; //!< observed line container
    typedef std::list< PlayerT > PlayerCont; //!< observed player container

private:

    GameTime M_time; //!< last updated time

    std::string M_their_team_name; //!< seen opponent team name

    //! marker ID map
    MarkerMap M_marker_map;
    //! marker ID map, old name
    MarkerMap M_marker_map_old;

    BallCont M_balls; //!< seen ball
    MarkerCont M_markers; //!< seen markers
    MarkerCont M_behind_markers; //!< seen behind markers
    LineCont M_lines; //!< seen lines

    PlayerCont M_teammates; //!< seen teammates
    PlayerCont M_unknown_teammates; //!< seen unknown teammates
    PlayerCont M_opponents; //!< seen opponents
    PlayerCont M_unknown_opponents; //!< seen unknown opponents
    PlayerCont M_unknown_players; //!< unknown players

public:

    /*!
      \brief create marker map
    */
    VisualSensor();

    /*!
      \brief analyze visual message and store analyzed data.
      \param msg message string
      \param team_name our team name
      \param version rcssserver protocol version
      \param current received game time.
    */
    void parse( const char * msg,
                const std::string & team_name, // self team name
                const double & version, // client version
                const GameTime & current );

    /*!
      \brief get observed opponent team name
      \return team name string
    */
    const std::string & theirTeamName() const
      {
          return M_their_team_name;
      }

    /*!
      \brief get last updated time
      \return game time object
    */
    const GameTime & time() const
      {
          return M_time;
      }

    /*!
      \brief get observed ball info
      \return const reference to the ball container
    */
    const BallCont & balls() const
      {
          return M_balls;
      }

    /*!
      \brief get observed marker(flag) info
      \return const reference to the marker container
    */
    const MarkerCont & markers() const
      {
          return M_markers;
      }

    /*!
      \brief get observed behind marker(flag) info
      \return const reference to the marker container
    */
    const MarkerCont & behindMarkers() const
      {
          return M_behind_markers;
      }

    /*!
      \brief get observed line info
      \return const reference to the line container
    */
    const LineCont & lines() const
      {
          return M_lines;
      }

    /*!
      \brief get observed teammate info
      \return const reference to the player container
    */
    const PlayerCont & teammates() const
      {
          return M_teammates;
      }

    /*!
      \brief get observed unknown teammate info
      \return const reference to the player container
    */
    const PlayerCont & unknownTeammates() const
      {
          return M_unknown_teammates;
      }

    /*!
      \brief get observed opponent info
      \return const reference to the player container
    */
    const PlayerCont & opponents() const
      {
          return M_opponents;
      }

    /*!
      \brief get observed unknown opponent info
      \return const reference to the player container
    */
    const PlayerCont & unknownOpponents() const
      {
          return M_unknown_opponents;
      }

    /*!
      \brief get observed completely unknown player info
      \return const reference to the player container
    */
    const PlayerCont & unknownPlayers() const
      {
          return M_unknown_players;
      }

    /*!
      \brief put data to ostream
      \param os reference to the output stream
      \return reference to the output stream
    */
    std::ostream & print( std::ostream & os );

private:
    //! get object type from identifire character.
    ObjectType getObjectTypeOf( const char c )
      {
          switch ( c ) {
          case 'f':  return Obj_Marker;
          case 'g':  return Obj_Goal;
          case 'F':  return Obj_Marker_Behind;
          case 'G':  return Obj_Goal_Behind;
          case 'p':  case 'P':  return Obj_Player;
          case 'b':  case 'B':  return Obj_Ball;
          case 'l':  return Obj_Line;
          default:   return Obj_Unknown;
          }
      }

    /*!
      \brief parse marker flag info
      \param tok pointer to the top of object info
      \param version rcssserver protocol version
      \param info pointer to the varialbe to store the data.

      get positional data from object info token
    */
    bool parseMarker( const char * tok,
                      const double version,
                      MarkerT * info );

    /*!
      \brief parse line info
      \param tok pointer to the top of object info
      \param version rcssserver protocol version
      \param info pointer to the varialbe to store the data.

      get positional data from object info token
    */
    bool parseLine( const char * tok,
                    const double & version,
                    LineT * info );

    /*!
      \brief parse line info
      \param tok pointer to the top of object info
      \param info pointer to the varialbe to store the data.

      get positional data from object info token
    */
    bool parseBall( const char * tok,
                    BallT * info );

    /*!
      \brief parse player info
      \param tok pointer to the top of object info
      \param team_name our team name
      \param info pointer to the varialbe to store the data.

      get positional data from object info token
    */
    PlayerInfoType parsePlayer( const char * tok,
                                const std::string & team_name,
                                PlayerT * info );

    /*!
      \brief reset all data. called just before new parsing process
    */
    void clearAll();

};

}

#endif
