// -*-c++-*-

/*!
  \file cycle_data.h
  \brief global cycle information class Header File
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

#ifndef RCSC_COACH_CYCLE_DATA_H
#define RCSC_COACH_CYCLE_DATA_H

#include <rcsc/geom/vector_2d.h>
#include <rcsc/game_time.h>
#include <rcsc/game_mode.h>
#include <rcsc/types.h>

#include <vector>

namespace rcsc {

class GlobalVisualSensor;

/*!
  \class CycleData
  \brief one cycle global field info snapshot
*/
class CycleData {
public:

    // all positional data from server is left-hand oriented

    /*!
      \struct BallT
      \brief global ball info.
    */
    struct BallT {
        Vector2D pos_; //!< global position
        Vector2D vel_; //!< velocity

        /*!
          \brief reverse all positional info to swap left/right
        */
        void reverse()
          {
              pos_ *= -1.0;
              vel_ *= -1.0;
          }
    };

    /*!
      \struct PlayerT
      \brief global player info
     */
    struct PlayerT {
        int unum_; //!< uniform number
        Vector2D pos_; //!< global position
        Vector2D vel_; //!< velocity
        double body_; //!< body angle
        double neck_; //!< neck angle relative to body
        Vector2D pointto_pos_; //!< pointing point
        bool goalie_; //!< if true, this plaeyr is goalie
        bool tackle_; //!< if true, this plaeyr is tackling

        /*!
          \brief initialize wiwth invalid values
        */
        PlayerT()
            : unum_( 0 )
            , pos_( Vector2D::INVALID )
            , vel_( Vector2D::INVALID )
            , body_( 0.0 )
            , neck_( 0.0 )
            , pointto_pos_( Vector2D::INVALID )
            , goalie_( false )
            , tackle_( false )
          { }

        /*!
          \brief put data to the output stream
          \param os reference to the output stream
          \return reference to the output stream
         */
        std::ostream & print( std::ostream & os ) const;
    };


    typedef std::vector< PlayerT > PlayerCont; //!< player data container type

private:

    //! game time of this cycle
    GameTime M_time;

    //! game mode
    GameMode M_game_mode;

    //! ball data
    BallT M_ball;

    //! teammate player set
    PlayerCont M_players_left;

    //! opponent player set
    PlayerCont M_players_right;


public:

    /*!
      \brief init variable with initial game status
    */
    CycleData()
        : M_time( -1, 0 )
      { }

    //////////////////////////////////////////////

    /*!
      \brief set data using sensory information
      \param see_global analyzed sensory visual data
      \param game_mode analyzed current game status
      \param current current game time
    */
    void assign( const GlobalVisualSensor & see_global,
                 const GameMode & game_mode,
                 const GameTime & current );

    //////////////////////////////////////////////

    /*!
      \brief get game cycle
      \return const reference to the GameTime object
     */
    const
    GameTime & time() const
      {
          return M_time;
      }

    /*!
      \brief get server playmode
      \return playmode type
    */
    const
    GameMode & gameMode() const
      {
          return M_game_mode;
      }

    /*!
      \brief get ball info
      \return const reference to the ball object
    */
    const
    BallT & ball() const
      {
          return M_ball;
      }

    /*!
      \brief get left players container
      \return const reference to the player object container
    */
    const
    PlayerCont & playersLeft() const
      {
          return M_players_left;
      }

    /*!
      \brief get right team players container
      \return const reference to the player object container
    */
    const
    PlayerCont & playersRight() const
      {
          return M_players_right;
      }

    //////////////////////////////////////////////

    /*!
      \brief calculate the offside line x for the left team (= right team's defense line)
      \return offside line x value
     */
    double getOffsideLineForLeft() const;

    /*!
      \brief calculate the offside line x for the right team (= right team's defense line)
      \return offside line x value
     */
    double getOffsideLineForRight() const;

    /*!
      \brief calculate the offside line x for the specified side.
      \return offside line x value
     */
    double getOffsideLineX( const SideID my_side ) const;

    /*!
      \brief get the current ball position status
      \return ball status type
     */
    BallStatus getBallStatus() const;

    /*!
      \brief get player nearest to the point
      \param players candidate player container
      \param point target point
      \return const pointer to the player object
    */
    const
    PlayerT * getPlayerNearestTo( const PlayerCont & players,
                                  const Vector2D & point ) const;

    /*!
      \brief get left player nearest to the point
      \param point target point
      \return const pointer to the player object
    */
    const
    PlayerT * getLeftPlayerNearestTo( const Vector2D & point ) const
      {
          return getPlayerNearestTo( M_players_left, point );
      }

    /*!
      \brief get left player nearest to the point
      \param point target point
      \return const pointer to the player object
    */
    const
    PlayerT * getRightPlayerNearestTo( const Vector2D & point ) const
      {
          return getPlayerNearestTo( M_players_right, point );
      }

    /*!
      \brief put all data to the output stream
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & print( std::ostream & os ) const;
};

}

#endif
