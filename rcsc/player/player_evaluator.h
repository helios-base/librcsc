// -*-c++-*-

/*!
  \file player_evaluator.h
  \brief player evaluator classes
*/

/*
 *Copyright:

 Copyright (C) Hiroki SHIMORA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3, or (at your option)
 any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////


#ifndef RCSC_PLAYER_PLAYER_EVALUATOR_H
#define RCSC_PLAYER_PLAYER_EVALUATOR_H

#include <rcsc/player/abstract_player_object.h>
#include <rcsc/player/world_model.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/common/server_param.h>

#include <memory>
#include <cmath>

namespace rcsc {

/*!
  \class PlayerEvaluator
  \brief abstract player evaluator function object class
 */
class PlayerEvaluator {
public:
    typedef std::shared_ptr< PlayerEvaluator > Ptr;
    typedef std::shared_ptr< const PlayerEvaluator > ConstPtr;

protected:
    /*!
      \brief protected constructor
     */
    PlayerEvaluator()
      { }
public:
    /*!
      \brief virtual destructor
     */
    virtual
    ~PlayerEvaluator()
      { }

    /*!
      \brief evaluation function
      \param p const reference to the target player object
      \return the evaluated value
     */
    virtual
    double operator()( const AbstractPlayerObject & p ) const = 0;
};

/*!
  \class AbsYDiffPlayerEvaluator
  \brief evaluation by y-coodinate difference
 */
class AbsYDiffPlayerEvaluator
    : public PlayerEvaluator {
private:
    //! base point
    const Vector2D M_point;

    // not used
    AbsYDiffPlayerEvaluator() = delete;
public:
    /*!
      \brief construct with base point
      \param point base point
     */
    AbsYDiffPlayerEvaluator( const Vector2D & point )
        : M_point( point )
      { }

    /*!
      \brief evaluation function
      \param p const reference to the target player
      \return evaluation value (absolute y-coodinate difference)
     */
    double operator()( const AbstractPlayerObject & p ) const
      {
          return std::fabs( p.pos().y - M_point.y );
      }
};

/*!
  \class AbsAngleDiffPlayerEvaluator
  \brief evaluation by absolute angle difference
 */
class AbsAngleDiffPlayerEvaluator
    : public PlayerEvaluator {
private:
    //! base point
    const Vector2D M_base_point;
    //! compared angle
    const AngleDeg M_base_angle;

    // not used
    AbsAngleDiffPlayerEvaluator() = delete;
public:
    /*!
      \brief construct with base point & angle
      \param base_point base point
      \param base_angle compared angle
     */
    AbsAngleDiffPlayerEvaluator( const Vector2D & base_point,
                                 const AngleDeg & base_angle )
        : M_base_point( base_point ),
          M_base_angle( base_angle )
      { }

    /*!
      \brief evaluation function
      \param p const reference to the target player
      \return evaluation value (absolute angle difference)
     */
    double operator()( const AbstractPlayerObject & p ) const
      {
          return ( ( p.pos() - M_base_point ).th() - M_base_angle ).abs();
      }
};

/*!
  \class XPosPlayerEvaluator
  \brief evaluation by x-coordinate value
 */
class XPosPlayerEvaluator
    : public PlayerEvaluator {
public:
    /*!
      \brief evaluation function
      \param p const reference to the target player
      \return evaluation value (x-coordinate value)
     */
    double operator()( const AbstractPlayerObject & p ) const
      {
          return p.pos().x;
      }
};

/*!
  \class DistFromPosPlayerEvaluator
  \brief evaluation by distance from position
 */
class DistFromPosPlayerEvaluator
    : public PlayerEvaluator {
private:
    //! base point
    const Vector2D M_base_point;
public:
    /*!
      \brief construct with base point
      \param base_point base point
     */
    DistFromPosPlayerEvaluator( const Vector2D & base_point )
        : M_base_point( base_point )
      { }

public:
    /*!
      \brief evaluation function
      \param p const reference to the target player
      \return evaluation value (distance from point)
     */
    double operator()( const AbstractPlayerObject & p ) const
      {
          return p.pos().dist( M_base_point );
      }
};

/*!
  \class SquaredDistFromPosPlayerEvaluator
  \brief evaluation by squared distance from position
 */
class SquaredDistFromPosPlayerEvaluator
    : public PlayerEvaluator {
private:
    //! base point
    const Vector2D M_base_point;
public:
    /*!
      \brief construct with base point
      \param base_point base point
    */
    SquaredDistFromPosPlayerEvaluator( const Vector2D & base_point )
        : M_base_point( base_point )
      { }

public:
    /*!
      \brief evaluation function
      \param p const reference to the target player
      \return evaluation value (squared distance from point)
     */
    double operator()( const AbstractPlayerObject & p ) const
      {
          return p.pos().dist2( M_base_point );
      }
};

/*!
  \class BallControllableDistancePlayerEvaluator
  \brief evaluation by ball controllable area
 */
class BallControllableDistancePlayerEvaluator
    : public PlayerEvaluator {

private:
    SideID M_our_side;

public:
    BallControllableDistancePlayerEvaluator( const WorldModel & wm )
        : M_our_side( wm.ourSide() )
      { }

    /*!
      \brief evaluation function
      \param p const reference to the target player
      \return evaluation value (controllable dinstance)
     */
    double operator()( const AbstractPlayerObject & p ) const
      {
          const double penalty_area_abs_x
              = ServerParam::i().pitchHalfLength() - ServerParam::i().penaltyAreaLength();

          if ( p.goalie()
               && p.pos().absY() < ServerParam::i().penaltyAreaHalfWidth()
               && ( ( p.side() == M_our_side && p.pos().x <= (- penalty_area_abs_x) )
                    || ( p.side() == NEUTRAL && p.pos().absX() <= penalty_area_abs_x )
                    || ( p.pos().x >= (+ penalty_area_abs_x) ) ) )
          {
              return ServerParam::i().catchableArea();
          }

          return ( p.playerTypePtr()
                   ? p.playerTypePtr()->kickableArea()
                   : ServerParam::i().defaultKickableArea() );
      }
};

/*!
  \class PlayerEvaluatorComparator
  \brief compare evaluated values of two player objects
 */
class PlayerEvaluatorComparator {
private:
    //! evaluator to evaluate each player
    PlayerEvaluator::ConstPtr M_evaluator;

    // not used
    PlayerEvaluatorComparator() = delete;
public:
    /*!
      \brief constructor
      \param evaluator dynamically allocated evaluator object
      to evaluate each player
    */
    explicit
    PlayerEvaluatorComparator( const PlayerEvaluator * evaluator )
        : M_evaluator( PlayerEvaluator::ConstPtr( evaluator ) )
      { }

    /*!
      \brief compare two players
      \param lhs a player to check
      \param rhs a player to check
      \return true if evaluation value of lhs is less than rhs
    */
    bool operator()( const AbstractPlayerObject * lhs,
                     const AbstractPlayerObject * rhs ) const
      {
          return (*M_evaluator)( *lhs ) < (*M_evaluator)( *rhs );
      }
};

}

#endif
