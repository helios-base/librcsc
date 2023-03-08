// -*-c++-*-

/*!
  \file localization.h
  \brief localization module Header File
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

#ifndef RCSC_PLAYER_LOCALIZER_H
#define RCSC_PLAYER_LOCALIZER_H

#include <rcsc/player/visual_sensor.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/game_time.h>
#include <rcsc/types.h>

namespace rcsc {

class ActionEffector;
class BodySensor;
class WorldModel;

/*!
  \class Localization
  \brief localization module
*/
class Localization {
public:

    /*!
      \brief localized player info
    */
    struct PlayerT {
        SideID side_; //!< side id
        int unum_; //!< uniform number
        bool goalie_; //!< true if goalie
        Vector2D pos_; //!< global coordinate
        Vector2D rpos_; //!< relative coordinate
        Vector2D vel_; //!< global velocity
        double body_; //!< body angle
        double face_; //!< face angle
        bool has_face_; //!< true if face angle is seen
        double arm_; //!< global pointing angle
        bool pointto_; //!< true if pointing is seen
        bool kicking_; //!< true if player performed a kick
        bool tackle_; //!< true if tackling is seen

        double dist_error_; //!< seen distance error

        /*!
          \brief init member variables by error value
        */
        PlayerT()
            : side_( NEUTRAL ),
              unum_( Unum_Unknown ),
              goalie_( false ),
              pos_( Vector2D::INVALIDATED ),
              rpos_( Vector2D::INVALIDATED ),
              vel_( Vector2D::INVALIDATED ),
              body_( 0.0 ),
              face_( 0.0 ),
              has_face_( false ),
              arm_( 0.0 ),
              pointto_( false ),
              kicking_( false ),
              tackle_( false ),
              dist_error_( 0.0 )
          { }

        /*!
          \brief reset all data
        */
        void reset()
          {
              pos_.invalidate();
              rpos_.invalidate();
              unum_ = Unum_Unknown;
              has_face_ = pointto_ = kicking_ = tackle_ = false;
              dist_error_ = 0.0;
          }

        /*!
          \brief check if velocity is estimated
          \return true if this player has velocity info
        */
        bool hasVel() const
          {
              return vel_.isValid();
          }

        /*!
          \brief check if angle is estimated
          \return true if this player has angle info
        */
        bool hasAngle() const
          {
              return has_face_;
          }

        /*!
          \brief check if this player is pointing
          \return true if this player is pointing somewhere
        */
        bool isPointing() const
          {
              return pointto_;
          }

        /*!
          \brief check if this player performed a kick.
          \return true if this player performed a kick.
         */
        bool isKicking() const
          {
              return kicking_;
          }

        /*!
          \brief check if this player is tackling
          \return true if this player is freezed by tackle effect
        */
        bool isTackling() const
          {
              return tackle_;
          }
    };

private:

    // not used
    Localization( const Localization & ) = delete;
    Localization & operator=( const Localization & ) = delete;

protected:

    /*!
      \brief default constructor (protected)
    */
    Localization() = default;

public:

    /*!
      \brief destructor
    */
    virtual
    ~Localization() = default;

   /*!
      \brief update internal state using received sense_body information
      \param body sense_body object
      \return result
     */
    virtual
    bool updateBySenseBody( const BodySensor & body ) = 0;

    /*!
      \brief estimate self facing direction.
      \param wm world model
      \param see analyzed see information
      \param self_face pointer to the result variable
      \param self_face_err pointer to the result variable
      \return result
     */
    virtual
    bool estimateSelfFace( const WorldModel & wm,
                           const VisualSensor & see,
                           double * self_face,
                           double * self_face_err ) = 0;

    /*!
      \brief localize self position.
      \param wm world model
      \param see analyzed see info
      \param act the last action info
      \param self_face localized face angle
      \param self_face_err localized face angle error
      \param self_pos pointer to the variable to store the localized self position
      \param self_pos_err pointer to the variable to store the localized self position error
      \return if failed, returns false
    */
    virtual
    bool localizeSelf( const WorldModel & wm,
                       const VisualSensor & see,
                       const ActionEffector & act,
                       const double self_face,
                       const double self_face_err,
                       Vector2D * self_pos,
                       Vector2D * self_pos_err ) = 0;

    /*!
      \brief localze ball relative info
      \param wm world model
      \param see analyzed see info
      \param self_face localized self face angle
      \param self_face_err localized self face angle error
      \param rpos pointer to the variable to store the localized relative position
      \param rpos_err pointer to the variable to store the localized relative position error
      \param rvel pointer to the variable to store the localized relative velocity
      \param rvel_err pointer to the variable to store the localized relative velocity error
      \return if failed, returns false
    */
    virtual
    bool localizeBallRelative( const WorldModel & wm,
                               const VisualSensor & see,
                               const double self_face,
                               const double self_face_err,
                               Vector2D * rpos,
                               Vector2D * rpos_err,
                               Vector2D * rvel,
                               Vector2D * rvel_err ) const = 0;

    /*!
      \brief localze other player
      \param wm world model
      \param from seen player info
      \param self_face localized self face angle
      \param self_face_err localized self face angle error
      \param self_pos localized self position
      \param self_vel localized self velocity
      \param to pointer to the variable to store the localized result
      \return if failed, returns false
    */
    virtual
    bool localizePlayer( const WorldModel & wm,
                         const VisualSensor::PlayerT & from,
                         const double self_face,
                         const double self_face_err,
                         const Vector2D & self_pos,
                         const Vector2D & self_vel,
                         PlayerT * to ) const = 0;
};

}

#endif
