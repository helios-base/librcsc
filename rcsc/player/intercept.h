// -*-c++-*-

/*!
  \file intercept.h
  \brief intercept action type Header File
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

#ifndef RCSC_PLAYER_INTERCEPT_H
#define RCSC_PLAYER_INTERCEPT_H

#include <rcsc/geom/vector_2d.h>

namespace rcsc {

/*-------------------------------------------------------------------*/

/*!
  \class Intercept
  \brief intercept action data
*/
class Intercept {
public:

    /*!
      \enum StaminaType
      \brief stamina type
    */
    enum StaminaType {
        NORMAL = 0, //!< ball gettable without stamina exhaust
        EXHAUST = 100, //!< fastest ball gettable, but recovery may be consumed.
    };

    /*!
      \enum ActionType
      \brief action type
    */
    enum ActionType {
        OMNI_DASH = 0,
        TURN_FORWARD_DASH = 1,
        TURN_BACK_DASH = 2,
        UNKNOWN_TYPE = 100,
    };

    static const double MIN_VALUE;

private:
    int M_index; //!< index value in all candidates
    double M_value; //!< evaluation value

    StaminaType M_stamina_type; //!< stamina type
    ActionType M_action_type; //!< action type

    int M_turn_step; //!< estimated turn step
    double M_turn_angle; //!< angle difference between current body angle and dash angle

    int M_dash_step; //!< estimated dash step
    double M_dash_power; //!< first dash power
    double M_dash_dir; //!< first dash direction (relative to body)

    Vector2D M_self_pos; //!< estimated final self position
    double M_ball_dist; //!< estimated final ball distance
    double M_stamina; //!< estimated final stamina value

public:

    /*!
      \brief create invalid info
    */
    Intercept()
        : M_index( -1 ),
          M_value( MIN_VALUE ),
          M_stamina_type( EXHAUST ),
          M_action_type( UNKNOWN_TYPE ),
          M_turn_step( 10000 ),
          M_turn_angle( 0.0 ),
          M_dash_step( 10000 ),
          M_dash_power( 100000.0 ),
          M_dash_dir( 0.0 ),
          M_self_pos( -10000.0, 0.0 ),
          M_ball_dist( 10000000.0 ),
          M_stamina( 0.0 )
    { }

    /*!
      \brief construct with all variables
    */
    Intercept( const StaminaType stamina_type,
               const ActionType action_type,
               const int turn_step,
               const double turn_angle,
               const int dash_step,
               const double dash_power,
               const double dash_dir,
               const Vector2D & self_pos,
               const double ball_dist,
               const double stamina )
        : M_index( -1 ),
          M_value( MIN_VALUE ),
          M_stamina_type( stamina_type ),
          M_action_type( action_type ),
          M_turn_step( turn_step ),
          M_turn_angle( turn_angle ),
          M_dash_step( dash_step ),
          M_dash_power( dash_power ),
          M_dash_dir( dash_dir ),
          M_self_pos( self_pos ),
          M_ball_dist( ball_dist ),
          M_stamina( stamina )
    { }

    /*!
      \brief set the evaluaion value with index count
      \param idx index value
      \param value evaluation value
    */
    void setEvaluation( const int idx,
                        const double value )
    {
        M_index = idx;
        M_value = value;
    }

    /*!
      \brief check if this object is legal one or not.
      \return checked result.
    */
    bool isValid() const
    {
        return M_action_type != UNKNOWN_TYPE;
    }

    /*!
      \brief get the index count within candidate list
      \return index count
     */
    int index() const
    {
        return M_index;
    }


    /*!
      \brief get the evaluation value of this intercept candidate
      \return evaluation value
    */
    double value() const
    {
        return M_value;
    }

    /*!
      \brief get the result stamina type
      \return stamina type id
    */
    StaminaType staminaType() const
    {
        return M_stamina_type;
    }

    /*!
      \brief get the action type
      \return type id
    */
    ActionType actionType() const
    {
        return M_action_type;
    }

    /*!
      \brief get estimated total turn steps
      \return the number of turn steps
    */
    int turnStep() const
    {
        return M_turn_step;
    }

    /*!
      \brief get the required first turn angle
      \return the first turn angle in degree
    */
    double turnAngle() const
    {
        return M_turn_angle;
    }

    /*!
      \brief get estimated total dash cycles
      \return the number of dash steps
    */
    int dashStep() const
    {
        return M_dash_step;
    }

    /*!
      \brief get esitimated total step to reach
      \return the number of total steps
    */
    int reachStep() const
    {
        return turnStep() + dashStep();
    }

    /*!
      \brief get dash power for the first dash
      \return dash power value
    */
    double dashPower() const
    {
        return M_dash_power;
    }

    /*!
      \brief ger the dash direction for the first dash
      \return dash direction value (relative to body)
    */
    double dashDir() const
    {
        return M_dash_dir;
    }

    /*!
      \brief get the estimated final self position
      \return final self position
    */
    const Vector2D & selfPos() const
    {
        return M_self_pos;
    }

    /*!
      \brief get the estimated final ball distance
      \return final ball distance
    */
    double ballDist() const
    {
        return M_ball_dist;
    }

    /*!
      \brief get the estimated final self stamina value
      \return final self stamina value
    */
    double stamina() const
    {
        return M_stamina;
    }

};

}

#endif
