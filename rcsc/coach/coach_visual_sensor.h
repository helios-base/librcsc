// -*-c++-*-

/*!
  \file coach_visual_sensor.h
  \brief coach/trainer visual sensor data holder class Header File
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

#ifndef RCSC_COACH_COACH_VISUAL_SENSOR_H
#define RCSC_COACH_COACH_VISUAL_SENSOR_H

#include <rcsc/coach/coach_ball_object.h>
#include <rcsc/coach/coach_player_object.h>
#include <rcsc/rcg/types.h>
#include <rcsc/game_time.h>

#include <string>
#include <iostream>

namespace rcsc {

/*!
  \class CoachVisualSensor
  \brief visual sensor for coach/trainer
 */
class CoachVisualSensor {
private:
    //! last updated time
    GameTime M_time;

    std::string M_team_name_left; //!< analyzed left team name string
    std::string M_team_name_right; //!< analyzed right team name string

    CoachBallObject M_ball; //!< analyzed ball info
    std::vector< CoachPlayerObject > M_players; //!< analyzed all players

public:
    /*!
      \brief init member variable
    */
    CoachVisualSensor();

    /*!
      \brief nothing to do
    */
    ~CoachVisualSensor();

    /*!
      \brief get updated time
      \return const reference to the game time object
    */
    const GameTime & time() const
      {
          return M_time;
      }

    /*!
      \brief get an analyzed left team name
      \return const reference to the string variable
     */
    const std::string & teamNameLeft() const
      {
          return M_team_name_left;
      }

    /*!
      \brief get an analyzed right team name
      \return const reference to the string variable
     */
    const std::string & teamNameRight() const
      {
          return M_team_name_right;
      }

    /*!
      \brief get analyzed ball info
      \return const reference to the analyzed data
    */
    const CoachBallObject & ball() const
      {
          return M_ball;
      }

    /*!
      \brief get seen players
      \return const reference to the player container
    */
    const std::vector< CoachPlayerObject > & players() const
      {
          return M_players;
      }

    /*!
      \brief analyze see message
      \param msg server raw message
      \param version client version
      \param current current game time
    */
    void parse( const char * msg,
                const double & version,
                const GameTime & current );

private:
    /*!
      \brief analyze see message. v6-
      \param msg server raw message
      \param our_teamname team name string
    */
    void parseV6( const char * msg );

    /*!
      \brief analyze see message. v7+
      \param msg server raw message
    */
    void parseV7( const char * msg );

public:

    /*!
      \brief put data to the output stream
      \param os reference to the output stream
      \return reference to the output stream
    */
    std::ostream & print( std::ostream & os ) const;

};

}

#endif
