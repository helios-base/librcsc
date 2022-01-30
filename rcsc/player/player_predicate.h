// -*-c++-*-

/*!
  \file player_predicate.h
  \brief player predicate classes Header File
*/

/*
 *Copyright:

 Copyright (C) Hiroki SHIMORA, Hidehisa AKIYAMA

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


#ifndef	RCSC_PLAYER_PLAYER_PREDICATE_H
#define	RCSC_PLAYER_PLAYER_PREDICATE_H

#include <rcsc/player/abstract_player_object.h>
#include <rcsc/player/world_model.h>
#include <rcsc/math_util.h>

#include <memory>
#include <vector>
#include <algorithm>
#include <cmath>

namespace rcsc {

/*!
  \class PlayerPredicate
  \brief abstract predicate class for player matching
*/
class PlayerPredicate {
public:
    typedef std::shared_ptr< PlayerPredicate > Ptr; //!< smart pointer type
    typedef std::shared_ptr< const PlayerPredicate > ConstPtr; //!< const smart pointer type

protected:
    /*!
      \brief protected constructor
    */
    PlayerPredicate()
      { }

public:
    /*!
      \brief virtual destructor
    */
    virtual
    ~PlayerPredicate()
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player is matched to the condition.
    */
    virtual
    bool operator()( const AbstractPlayerObject & p ) const = 0;

    /*!
      \brief create clone object.
      \return cloned object.
     */
    virtual
    Ptr clone() const = 0;
};

/*!
  \class AndPlayerPredicate
  \brief composite logical "and" predicate
*/
class AndPlayerPredicate
    : public PlayerPredicate {
private:
    //! the set of predicate
    std::vector< PlayerPredicate::ConstPtr > M_predicates;

    // not used
    AndPlayerPredicate() = delete;
public:
    // XXX: implicit_shared_ptr

    /*!
      \brief construct with 2 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
    */
    AndPlayerPredicate( const PlayerPredicate * p1 ,
                        const PlayerPredicate * p2 )
        : M_predicates()
      {
          M_predicates.reserve( 2 );
          M_predicates.push_back( ConstPtr( p1 ) );
          M_predicates.push_back( ConstPtr( p2 ) );
      }

    /*!
      \brief construct with 3 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
      \param p3 3rd predicate
    */
    AndPlayerPredicate( const PlayerPredicate * p1 ,
                        const PlayerPredicate * p2 ,
                        const PlayerPredicate * p3 )
        : M_predicates()
      {
          M_predicates.reserve( 3 );
          M_predicates.push_back( ConstPtr( p1 ) );
          M_predicates.push_back( ConstPtr( p2 ) );
          M_predicates.push_back( ConstPtr( p3 ) );
      }

    /*!
      \brief construct with 4 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
      \param p3 3rd predicate
      \param p4 4th predicate
    */
    AndPlayerPredicate( const PlayerPredicate * p1 ,
                        const PlayerPredicate * p2 ,
                        const PlayerPredicate * p3 ,
                        const PlayerPredicate * p4 )
        : M_predicates()
      {
          M_predicates.reserve( 4 );
          M_predicates.push_back( ConstPtr( p1 ) );
          M_predicates.push_back( ConstPtr( p2 ) );
          M_predicates.push_back( ConstPtr( p3 ) );
          M_predicates.push_back( ConstPtr( p4 ) );
      }

    /*!
      \brief construct with 5 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
      \param p3 3rd predicate
      \param p4 4th predicate
      \param p5 5th predicate
    */
    AndPlayerPredicate( const PlayerPredicate * p1 ,
                        const PlayerPredicate * p2 ,
                        const PlayerPredicate * p3 ,
                        const PlayerPredicate * p4 ,
                        const PlayerPredicate * p5 )
        : M_predicates()
      {
          M_predicates.reserve( 5 );
          M_predicates.push_back( ConstPtr( p1 ) );
          M_predicates.push_back( ConstPtr( p2 ) );
          M_predicates.push_back( ConstPtr( p3 ) );
          M_predicates.push_back( ConstPtr( p4 ) );
          M_predicates.push_back( ConstPtr( p5 ) );
      }

    /*!
      \brief construct with 6 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
      \param p3 3rd predicate
      \param p4 4th predicate
      \param p5 5th predicate
      \param p6 6th predicate
    */
    AndPlayerPredicate( const PlayerPredicate * p1 ,
                        const PlayerPredicate * p2 ,
                        const PlayerPredicate * p3 ,
                        const PlayerPredicate * p4 ,
                        const PlayerPredicate * p5 ,
                        const PlayerPredicate * p6 )
        : M_predicates()
      {
          M_predicates.reserve( 6 );
          M_predicates.push_back( ConstPtr( p1 ) );
          M_predicates.push_back( ConstPtr( p2 ) );
          M_predicates.push_back( ConstPtr( p3 ) );
          M_predicates.push_back( ConstPtr( p4 ) );
          M_predicates.push_back( ConstPtr( p5 ) );
          M_predicates.push_back( ConstPtr( p6 ) );
      }

    /*!
      \brief construct with 7 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
      \param p3 3rd predicate
      \param p4 4th predicate
      \param p5 5th predicate
      \param p6 6th predicate
      \param p7 7th predicate
    */
    AndPlayerPredicate( const PlayerPredicate * p1 ,
                        const PlayerPredicate * p2 ,
                        const PlayerPredicate * p3 ,
                        const PlayerPredicate * p4 ,
                        const PlayerPredicate * p5 ,
                        const PlayerPredicate * p6 ,
                        const PlayerPredicate * p7 )
        : M_predicates()
      {
          M_predicates.reserve( 7 );
          M_predicates.push_back( ConstPtr( p1 ) );
          M_predicates.push_back( ConstPtr( p2 ) );
          M_predicates.push_back( ConstPtr( p3 ) );
          M_predicates.push_back( ConstPtr( p4 ) );
          M_predicates.push_back( ConstPtr( p5 ) );
          M_predicates.push_back( ConstPtr( p6 ) );
          M_predicates.push_back( ConstPtr( p7 ) );
      }

    /*!
      \brief construct with 8 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
      \param p3 3rd predicate
      \param p4 4th predicate
      \param p5 5th predicate
      \param p6 6th predicate
      \param p7 7th predicate
      \param p8 8th predicate
    */
    AndPlayerPredicate( const PlayerPredicate * p1 ,
                        const PlayerPredicate * p2 ,
                        const PlayerPredicate * p3 ,
                        const PlayerPredicate * p4 ,
                        const PlayerPredicate * p5 ,
                        const PlayerPredicate * p6 ,
                        const PlayerPredicate * p7 ,
                        const PlayerPredicate * p8 )
        : M_predicates()
      {
          M_predicates.reserve( 8 );
          M_predicates.push_back( ConstPtr( p1 ) );
          M_predicates.push_back( ConstPtr( p2 ) );
          M_predicates.push_back( ConstPtr( p3 ) );
          M_predicates.push_back( ConstPtr( p4 ) );
          M_predicates.push_back( ConstPtr( p5 ) );
          M_predicates.push_back( ConstPtr( p6 ) );
          M_predicates.push_back( ConstPtr( p7 ) );
          M_predicates.push_back( ConstPtr( p8 ) );
      }

private:
    /*!
      \brief construct with children predicates container.
      \param predicates predicates container.
     */
    AndPlayerPredicate( const std::vector< ConstPtr > & predicates )
        : M_predicates( predicates )
      { }

public:

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return the result of "and" operation of all predicates
    */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          for ( const ConstPtr & pred : M_predicates )
          {
              if ( ! (*pred)( p ) )
              {
                  return false;
              }
          }

          return true;
      }

    /*!
      \brief create clone object.
      \return cloned object.
     */
    Ptr clone() const
      {
          return Ptr( new AndPlayerPredicate( M_predicates ) );
      }
};

/*!
  \class OrPlayerPredicate
  \brief composite logical "or" predicate
*/
class OrPlayerPredicate
    : public PlayerPredicate {
private:
    //! the set of predicate
    std::vector< ConstPtr > M_predicates;

    // not used
    OrPlayerPredicate() = delete;
public:
    // XXX: implicit_shared_ptr

    /*!
      \brief construct with 2 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
    */
    OrPlayerPredicate( const PlayerPredicate * p1 ,
                       const PlayerPredicate * p2 )
        : M_predicates()
      {
          M_predicates.push_back( ConstPtr( p1 ) );
          M_predicates.push_back( ConstPtr( p2 ) );
      }

    /*!
      \brief construct with 3 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
      \param p3 3rd predicate
    */
    OrPlayerPredicate( const PlayerPredicate * p1 ,
                       const PlayerPredicate * p2 ,
                       const PlayerPredicate * p3 )
        : M_predicates()
      {
          M_predicates.push_back( ConstPtr( p1 ) );
          M_predicates.push_back( ConstPtr( p2 ) );
          M_predicates.push_back( ConstPtr( p3 ) );
      }

    /*!
      \brief construct with 4 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
      \param p3 3rd predicate
      \param p4 4th predicate
    */
    OrPlayerPredicate( const PlayerPredicate * p1 ,
                       const PlayerPredicate * p2 ,
                       const PlayerPredicate * p3 ,
                       const PlayerPredicate * p4 )
        : M_predicates()
      {
          M_predicates.push_back( ConstPtr( p1 ) );
          M_predicates.push_back( ConstPtr( p2 ) );
          M_predicates.push_back( ConstPtr( p3 ) );
          M_predicates.push_back( ConstPtr( p4 ) );
      }

    /*!
      \brief construct with 5 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
      \param p3 3rd predicate
      \param p4 4th predicate
      \param p5 5th predicate
    */
    OrPlayerPredicate( const PlayerPredicate * p1 ,
                       const PlayerPredicate * p2 ,
                       const PlayerPredicate * p3 ,
                       const PlayerPredicate * p4 ,
                       const PlayerPredicate * p5 )
        : M_predicates()
      {
          M_predicates.push_back( ConstPtr( p1 ) );
          M_predicates.push_back( ConstPtr( p2 ) );
          M_predicates.push_back( ConstPtr( p3 ) );
          M_predicates.push_back( ConstPtr( p4 ) );
          M_predicates.push_back( ConstPtr( p5 ) );
      }

    /*!
      \brief construct with 6 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
      \param p3 3rd predicate
      \param p4 4th predicate
      \param p5 5th predicate
      \param p6 6th predicate
    */
    OrPlayerPredicate( const PlayerPredicate * p1 ,
                       const PlayerPredicate * p2 ,
                       const PlayerPredicate * p3 ,
                       const PlayerPredicate * p4 ,
                       const PlayerPredicate * p5 ,
                       const PlayerPredicate * p6 )
        : M_predicates()
      {
          M_predicates.push_back( ConstPtr( p1 ) );
          M_predicates.push_back( ConstPtr( p2 ) );
          M_predicates.push_back( ConstPtr( p3 ) );
          M_predicates.push_back( ConstPtr( p4 ) );
          M_predicates.push_back( ConstPtr( p5 ) );
          M_predicates.push_back( ConstPtr( p6 ) );
      }

    /*!
      \brief construct with 7 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
      \param p3 3rd predicate
      \param p4 4th predicate
      \param p5 5th predicate
      \param p6 6th predicate
      \param p7 7th predicate
    */
    OrPlayerPredicate( const PlayerPredicate * p1 ,
                       const PlayerPredicate * p2 ,
                       const PlayerPredicate * p3 ,
                       const PlayerPredicate * p4 ,
                       const PlayerPredicate * p5 ,
                       const PlayerPredicate * p6 ,
                       const PlayerPredicate * p7 )
        : M_predicates()
      {
          M_predicates.push_back( ConstPtr( p1 ) );
          M_predicates.push_back( ConstPtr( p2 ) );
          M_predicates.push_back( ConstPtr( p3 ) );
          M_predicates.push_back( ConstPtr( p4 ) );
          M_predicates.push_back( ConstPtr( p5 ) );
          M_predicates.push_back( ConstPtr( p6 ) );
          M_predicates.push_back( ConstPtr( p7 ) );
      }

    /*!
      \brief construct with 8 predicates. all arguments must be a dynamically allocated object.
      \param p1 1st predicate
      \param p2 2nd predicate
      \param p3 3rd predicate
      \param p4 4th predicate
      \param p5 5th predicate
      \param p6 6th predicate
      \param p7 7th predicate
      \param p8 8th predicate
    */
    OrPlayerPredicate( const PlayerPredicate * p1 ,
                       const PlayerPredicate * p2 ,
                       const PlayerPredicate * p3 ,
                       const PlayerPredicate * p4 ,
                       const PlayerPredicate * p5 ,
                       const PlayerPredicate * p6 ,
                       const PlayerPredicate * p7 ,
                       const PlayerPredicate * p8 )
        : M_predicates()
      {
          M_predicates.push_back( ConstPtr( p1 ) );
          M_predicates.push_back( ConstPtr( p2 ) );
          M_predicates.push_back( ConstPtr( p3 ) );
          M_predicates.push_back( ConstPtr( p4 ) );
          M_predicates.push_back( ConstPtr( p5 ) );
          M_predicates.push_back( ConstPtr( p6 ) );
          M_predicates.push_back( ConstPtr( p7 ) );
          M_predicates.push_back( ConstPtr( p8 ) );
      }

private:
    /*!
      \brief construct with children predicates container.
      \param predicates predicates container.
     */
    OrPlayerPredicate( const std::vector< ConstPtr > & predicates )
        : M_predicates( predicates )
      { }

public:

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return the result of "or" operation of all predicates
    */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          for ( const ConstPtr & pred : M_predicates )
          {
              if ( (*pred)( p ) )
              {
                  return true;
              }
          }

          return false;
      }

    /*!
      \brief create clone object.
      \return cloned object.
     */
    Ptr clone() const
      {
          return Ptr( new OrPlayerPredicate( M_predicates ) );
      }
};

/*!
  \class NotPlayerPredicate
  \brief logical "not" predicate
*/
class NotPlayerPredicate
    : public PlayerPredicate {
private:
    //! predicate instance
    ConstPtr M_predicate;

    // not used
    NotPlayerPredicate() = delete;
public:

    /*!
      \brief construct with the predicate. argument must be a dynamically allocated object.
      \param predicate pointer to the predicate instance
    */
    explicit
    NotPlayerPredicate( const PlayerPredicate * predicate )
        : M_predicate( predicate )
      { }

    /*!
      \brief construct with the predicate.
      \param predicate pointer to the predicate instance
    */
    explicit
    NotPlayerPredicate( ConstPtr predicate )
        : M_predicate( predicate )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return the locigal "not" result of M_predicate
    */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return ! (*M_predicate)( p );
      }

    /*!
      \brief create clone object.
      \return cloned object.
     */
    Ptr clone() const
      {
          return Ptr( new NotPlayerPredicate( M_predicate ) );
      }
};

/*!
  \class SelfPlayerPredicate
  \brief check if target player is self or not
*/
class SelfPlayerPredicate
    : public PlayerPredicate {
private:
    //! side self player belonging
    const SideID M_our_side;
    //! uniform number of self player
    const int M_self_unum;

    // not used
    SelfPlayerPredicate() = delete;
public:
    /*!
      \brief construct with the WorldModel instance
      \param wm const reference to the WorldModel instance
    */
    explicit
    SelfPlayerPredicate( const WorldModel & wm )
        : M_our_side( wm.ourSide() ),
          M_self_unum( wm.self().unum() )
      { }

    /*!
      \brief construct with side and uniform number of self
      \param our_side side self player belonging
      \param self_unum uniform number of self player
    */
    explicit
    SelfPlayerPredicate( const SideID our_side,
                         const int self_unum )
        : M_our_side( our_side ),
          M_self_unum( self_unum )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player is agent itself
    */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return p.side() == M_our_side
              && p.unum() == M_self_unum;
      }

    /*!
      \brief create clone object.
      \return cloned object.
     */
    Ptr clone() const
      {
          return Ptr( new SelfPlayerPredicate( M_our_side, M_self_unum ) );
      }
};

/*!
  \class TeammateOrSelfPlayerPredicate
  \brief check if target player is teammate (include self) or not
*/
class TeammateOrSelfPlayerPredicate
    : public PlayerPredicate {
private:
    //! side self player belonging
    const SideID M_our_side;

    // not used
    TeammateOrSelfPlayerPredicate() = delete;
public:
    /*!
      \brief construct with the WorldModel instance
      \param wm const reference to the WorldModel instance
    */
    explicit
    TeammateOrSelfPlayerPredicate( const WorldModel & wm )
        : M_our_side( wm.ourSide() )
      { }

    explicit
    TeammateOrSelfPlayerPredicate( const SideID our_side )
        : M_our_side( our_side )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player is teammate (include self)
    */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return p.side() == M_our_side;
      }


    /*!
      \brief create clone object.
      \return cloned object.
     */
    Ptr clone() const
      {
          return Ptr( new TeammateOrSelfPlayerPredicate( M_our_side ) );
      }
};

/*!
  \class TeammatePlayerPredicate
  \brief check if target player is teammate (not include self) or not
*/
class TeammatePlayerPredicate
    : public PlayerPredicate {
private:
    //! side self player belonging
    const SideID M_our_side;
    //! uniform number of self player
    const int M_self_unum;

    // not used
    TeammatePlayerPredicate() = delete;
public:
    /*!
      \brief construct with the WorldModel instance
      \param wm const reference to the WorldModel instance
    */
    explicit
    TeammatePlayerPredicate( const WorldModel & wm )
        : M_our_side( wm.ourSide() )
        , M_self_unum( wm.self().unum() )
      { }

    /*!
      \brief construct with side and uniform number of self
      \param our_side side self player belonging
      \param self_unum uniform number of self player
    */
    explicit
    TeammatePlayerPredicate( const SideID our_side,
                             const int self_unum )
        : M_our_side( our_side )
        , M_self_unum( self_unum )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player is teammate (not include self)
    */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return p.side() == M_our_side
              && p.unum() != M_self_unum;
      }

    /*!
      \brief create clone object.
      \return cloned object.
     */
    Ptr clone() const
      {
          return Ptr( new TeammatePlayerPredicate( M_our_side, M_self_unum ) );
      }
};

/*!
  \class OpponentPlayerPredicate
  \brief check if target player is opponent (not include unknown player) or not
*/
class OpponentPlayerPredicate
    : public PlayerPredicate {
private:
    //! side self player belonging
    const SideID M_our_side;

    // not used
    OpponentPlayerPredicate() = delete;
public:
    /*!
      \brief construct with the WorldModel instance
      \param wm const reference to the WorldModel instance
    */
    explicit
    OpponentPlayerPredicate( const WorldModel & wm )
        : M_our_side( wm.ourSide() )
      { }

    /*!
      \brief construct with side and uniform number of self
      \param our_side side self player belonging
    */
    explicit
    OpponentPlayerPredicate( const SideID our_side )
        : M_our_side( our_side )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player is opponent (not include unknown player)
    */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return p.side() != M_our_side
              && p.side() != NEUTRAL;
      }

    /*!
      \brief create clone object.
      \return cloned object.
     */
    Ptr clone() const
      {
          return Ptr( new OpponentPlayerPredicate( M_our_side ) );
      }
};

/*!
  \class OpponentOrUnknownPlayerPredicate
  \brief check if target player is opponent (include unknown player) or not
*/
class OpponentOrUnknownPlayerPredicate
    : public PlayerPredicate {
private:
    //! side self player belonging
    const SideID M_our_side;

    // not used
    OpponentOrUnknownPlayerPredicate() = delete;
public:
    /*!
      \brief construct with the WorldModel instance
      \param wm const reference to the WorldModel instance
    */
    explicit
    OpponentOrUnknownPlayerPredicate( const WorldModel & wm )
        : M_our_side( wm.ourSide() )
      { }

    /*!
      \brief construct with side and uniform number of self
      \param our_side side self player belonging
    */
    explicit
    OpponentOrUnknownPlayerPredicate( const SideID our_side )
        : M_our_side( our_side )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player is opponent (include unknown player)
    */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return p.side() != M_our_side;
      }

    /*!
      \brief create clone object.
      \return cloned object.
     */
    Ptr clone() const
      {
          return Ptr( new OpponentOrUnknownPlayerPredicate( M_our_side ) );
      }
};

/*!
  \class GoaliePlayerPredicate
  \brief check if target player is goalie or not
*/
class GoaliePlayerPredicate
    : public PlayerPredicate {
public:

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player is goalie
    */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return p.goalie();
      }

    /*!
      \brief create clone object.
      \return cloned object.
     */
    Ptr clone() const
      {
          return Ptr( new GoaliePlayerPredicate() );
      }
};

/*!
  \class FieldPlayerPredicate
  \brief check if target player is field player or not
*/
class FieldPlayerPredicate
    : public PlayerPredicate {
public:

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player is not goalie
    */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return ! p.goalie();
      }

    /*!
      \brief create clone object.
      \return cloned object.
     */
    Ptr clone() const
      {
          return Ptr( new FieldPlayerPredicate() );
      }
};

/*!
  \class CoordinateAccuratePlayerPredicate
  \brief check if target player's positional information has enough accuracy.
*/
class CoordinateAccuratePlayerPredicate
    : public PlayerPredicate {
private:
    //! threshold accuracy value
    const int M_threshold;

    // not used
    CoordinateAccuratePlayerPredicate() = delete;
public:
    /*!
      \brief construct with threshold value
      \param threshold accuracy threshold value
    */
    explicit
    CoordinateAccuratePlayerPredicate( const int threshold )
        : M_threshold( threshold )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player's posCount() is less than equal threshold
    */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return p.posCount() <= M_threshold;
      }

    /*!
      \brief create clone object.
      \return cloned object.
     */
    Ptr clone() const
      {
          return Ptr( new CoordinateAccuratePlayerPredicate( M_threshold ) );
      }
};

/*!
  \class GhostPlayerPredicate
  \brief check if target player is ghost object or not
*/
class GhostPlayerPredicate
    : public PlayerPredicate {
private:

public:

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player is a ghost object.
    */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return p.isGhost();
      }

    /*!
      \brief create clone object.
      \return cloned object.
     */
    Ptr clone() const
      {
          return Ptr( new GhostPlayerPredicate() );
      }
};

/*!
  \class NoGhostPlayerPredicate
  \brief check if target player is not ghost and has enough accuracy.
*/
class NoGhostPlayerPredicate
    : public PlayerPredicate {
private:
    //! threshold accuracy value
    const int M_threshold;

    // not used
    NoGhostPlayerPredicate() = delete;
public:
    /*!
      \brief construct with threshold value
      \param threshold accuracy threshold value
    */
    explicit
    NoGhostPlayerPredicate( const int threshold )
        : M_threshold( threshold )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player's posCount() is less than equal threshold
    */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return ! p.isGhost()
              && p.posCount() <= M_threshold;
      }

    /*!
      \brief create clone object.
      \return cloned object.
     */
    Ptr clone() const
      {
          return Ptr( new NoGhostPlayerPredicate( M_threshold ) );
      }
};

/*!
  \class XCoordinateForwardPlayerPredicate
  \brief check if target player's x coordinate is greater(forwarder) than threshold value
*/
class XCoordinateForwardPlayerPredicate
    : public PlayerPredicate {
private:
    //! threshold x-coordinate value
    const double M_threshold;

    // not used
    XCoordinateForwardPlayerPredicate() = delete;
public:
    /*!
      \brief construct with threshold value
      \param threshold x-coordinate threshold value
    */
    explicit
    XCoordinateForwardPlayerPredicate( const double & threshold )
        : M_threshold( threshold )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player's pos(.x) is greater(forwarder) than equal threshold
    */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return p.pos().x >= M_threshold;
      }

    /*!
      \brief create clone object.
      \return cloned object.
     */
    Ptr clone() const
      {
          return Ptr( new XCoordinateForwardPlayerPredicate( M_threshold ) );
      }
};

/*!
  \class XCoordinateBackwardPlayerPredicate
  \brief check if target player's x coordinate is less(backwarder) than threshold value
*/
class XCoordinateBackwardPlayerPredicate
    : public PlayerPredicate {
private:
    //! threshold x-coordinate value
    const double M_threshold;

    // not used
    XCoordinateBackwardPlayerPredicate() = delete;
public:
    /*!
      \brief construct with threshold value
      \param threshold x-coordinate threshold value
    */
    explicit
    XCoordinateBackwardPlayerPredicate( const double & threshold )
        : M_threshold( threshold )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player's pos().x is less(backwarder) than equal threshold
    */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return p.pos().x <= M_threshold;
      }

    /*!
      \brief create clone object.
      \return cloned object.
     */
    Ptr clone() const
      {
          return Ptr( new XCoordinateBackwardPlayerPredicate( M_threshold ) );
      }
};

/*!
  \class YCoordinatePlusPlayerPredicate
  \brief check if target player's y coordinate is more right than threshold value
*/
class YCoordinatePlusPlayerPredicate
    : public PlayerPredicate {
private:
    //! threshold y-coordinate value
    const double M_threshold;

    // not used
    YCoordinatePlusPlayerPredicate() = delete;
public:
    /*!
      \brief construct with threshold value
      \param threshold y-coordinate threshold value
    */
    explicit
    YCoordinatePlusPlayerPredicate( const double & threshold )
        : M_threshold( threshold )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player's pos().y is more right than equal threshold
    */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return p.pos().y >= M_threshold;
      }

    /*!
      \brief create clone object.
      \return cloned object.
     */
    Ptr clone() const
      {
          return Ptr( new YCoordinatePlusPlayerPredicate( M_threshold ) );
      }
};

/*!
  \class YCoordinateMinusPlayerPredicate
  \brief check if target player's y coordinate is more left than threshold value
*/
class YCoordinateMinusPlayerPredicate
    : public PlayerPredicate {
private:
    //! threshold y-coordinate value
    const double M_threshold;

    // not used
    YCoordinateMinusPlayerPredicate() = delete;
public:
    /*!
      \brief construct with threshold value
      \param threshold y-coordinate threshold value
    */
    explicit
    YCoordinateMinusPlayerPredicate( const double & threshold )
        : M_threshold( threshold )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player's pos().y is more left than equal threshold
    */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return p.pos().y <= M_threshold;
      }

    /*!
      \brief create clone object.
      \return cloned object.
     */
    Ptr clone() const
      {
          return Ptr( new YCoordinateMinusPlayerPredicate( M_threshold ) );
      }
};

/*!
  \class PointFarPlayerPredicate
  \brief check if target player's distance from tha base point is greater than threshold distance
*/
class PointFarPlayerPredicate
    : public PlayerPredicate {
private:
    //! base point
    const Vector2D M_base_point;
    //! squared threshold distance
    const double M_threshold2;

    // not used
    PointFarPlayerPredicate() = delete;
public:
    /*!
      \brief construct with base point and threshold distance
      \param base_point base point
      \param threshold distance threshold value
    */
    PointFarPlayerPredicate( const Vector2D & base_point,
                             const double & threshold )
        : M_base_point( base_point )
        , M_threshold2( threshold * threshold )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player's distance from base_point is greater than equal threshold
    */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return ( p.pos() - M_base_point ).r2() >= M_threshold2;
      }

    /*!
      \brief create clone object.
      \return cloned object.
     */
    Ptr clone() const
      {
          return Ptr( new PointFarPlayerPredicate( M_base_point, M_threshold2 ) );
      }
};

/*!
  \class PointNearPlayerPredicate
  \brief check if target player's distance from tha base point is less than threshold distance
*/
class PointNearPlayerPredicate
    : public PlayerPredicate {
private:
    //! base point
    const Vector2D M_base_point;
    //! squared threshold distance
    const double M_threshold2;

    // not used
    PointNearPlayerPredicate() = delete;
public:
    /*!
      \brief construct with base point and threshold distance
      \param base_point base point
      \param threshold distance threshold value
    */
    PointNearPlayerPredicate( const Vector2D & base_point,
                              const double & threshold )
        : M_base_point( base_point ),
          M_threshold2( threshold * threshold )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player's distance from base_point is less than equal threshold
    */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return ( p.pos() - M_base_point ).r2() <= M_threshold2;
      }

    /*!
      \brief create clone object.
      \return cloned object.
     */
    Ptr clone() const
      {
          return Ptr( new PointNearPlayerPredicate( M_base_point, M_threshold2 ) );
      }
};

/*!
  \class AbsAngleDiffLessPlayerPredicate
  \brief check if target player's absolute angle difference from base angle is less than threshold angle
*/
class AbsAngleDiffLessPlayerPredicate
    : public PlayerPredicate {
private:
    //! base point
    const Vector2D M_base_point;
    //! compared angle
    const AngleDeg M_base_angle;
    //! angle threshold value (degree)
    const double M_threshold;

    // not used
    AbsAngleDiffLessPlayerPredicate() = delete;
public:
    /*!
      \brief construct with base point and threshold distance
      \param base_point base point
      \param base_angle compared angle
      \param degree_threshold angle threshold value (degree)
    */
    AbsAngleDiffLessPlayerPredicate( const Vector2D & base_point,
                                     const AngleDeg & base_angle,
                                     const double & degree_threshold )
        : M_base_point( base_point ),
          M_base_angle( base_angle ),
          M_threshold( std::fabs( degree_threshold ) )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player's absolute angle difference from base_angle is less than equal threshold
    */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return ( ( p.pos() - M_base_point ).th() - M_base_angle ).abs() <= M_threshold;
      }

    /*!
      \brief create clone object.
      \return cloned object.
     */
    Ptr clone() const
      {
          return Ptr( new AbsAngleDiffLessPlayerPredicate( M_base_point, M_base_angle, M_threshold ) );
      }
};

/*!
  \class AbsAngleDiffGreaterPlayerPredicate
  \brief check if target player's absolute angle difference from base angle is greater than threshold angle
*/
class AbsAngleDiffGreaterPlayerPredicate
    : public PlayerPredicate {
private:
    //! base point
    const Vector2D M_base_point;
    //! compared angle
    const AngleDeg M_base_angle;
    //! angle threshold value (degree)
    const double M_threshold;

    // not used
    AbsAngleDiffGreaterPlayerPredicate() = delete;
public:
    /*!
      \brief construct with base point and threshold distance
      \param base_point base point
      \param base_angle compared angle
      \param threshold angle threshold value (degree)
    */
    AbsAngleDiffGreaterPlayerPredicate( const Vector2D & base_point,
                                        const AngleDeg & base_angle,
                                        const double & threshold )
        : M_base_point( base_point ),
          M_base_angle( base_angle ),
          M_threshold( std::fabs( threshold ) )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player's absolute angle difference from base_angle is greater than equal threshold
    */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return ( ( p.pos() - M_base_point ).th() - M_base_angle ).abs() >= M_threshold;
      }

    /*!
      \brief create clone object.
      \return cloned object.
     */
    Ptr clone() const
      {
          return Ptr( new AbsAngleDiffGreaterPlayerPredicate( M_base_point, M_base_angle, M_threshold ) );
      }
};

/*!
  \class OffsidePositionPlayerPredicate
  \brief check if target player is in offside area
*/
class OffsidePositionPlayerPredicate
    : public PlayerPredicate {
private:
    //! const reference to the WorldModel instance
    const WorldModel & M_world;

    // not used
    OffsidePositionPlayerPredicate() = delete;
public:
    /*!
      \brief construct with the WorldModel instance
      \param wm const reference to the WorldModel instance
    */
    OffsidePositionPlayerPredicate( const WorldModel & wm )
        : M_world( wm )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player is in offside area. if target player is unknown, false is always returned.
    */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          if ( p.side() == M_world.self().side() )
          {
              return p.pos().x > M_world.offsideLineX();
          }
          else if ( p.side() == NEUTRAL )
          {
              return false;
          }
          else
          {
              return p.pos().x < bound( M_world.ball().pos().x, M_world.ourDefenseLineX(), 0.0 );
          }
      }

    /*!
      \brief create clone object.
      \return cloned object.
     */
    Ptr clone() const
      {
          return Ptr( new OffsidePositionPlayerPredicate( M_world ) );
      }
};

/*!
  \class ExistNearPlayerPlayerPredicate
  \brief check if a player exists within the specified distance from the filtered player.
*/
class ExistNearPlayerPlayerPredicate
    : public PlayerPredicate {

private:
    //! const reference to the WorldModel instance
    const rcsc::WorldModel & M_world;

    //! predicate to check players around of the argument player.
    ConstPtr M_predicate;

    //! squared circle radius
    double M_threshold2;

public:
    /*!
      \brief constructor
      \param wm const reference to the WorldModel instance
      \param predicate filter predicate object
      \param threshold distance
    */
    ExistNearPlayerPlayerPredicate( const WorldModel & wm,
                                    ConstPtr predicate,
                                    const double & threshold )
        : M_world( wm ),
          M_predicate( predicate ),
          M_threshold2( threshold * threshold )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player is within threshold distance from the filtered player.
    */
    bool operator() ( const AbstractPlayerObject & p ) const
      {
          for ( const AbstractPlayerObject * target : M_world.allPlayers() )
          {
              if ( ( target->pos() - p.pos() ).r2() <= M_threshold2
                   && (*M_predicate)( *target ) )
              {
                  return true;
              }
          }

          return false;
      }

    /*!
      \brief create clone object.
      \return cloned object.
     */
    Ptr clone() const
      {
          return Ptr( new ExistNearPlayerPlayerPredicate( M_world,
                                                          M_predicate->clone(),
                                                          std::sqrt( M_threshold2 ) ) );
      }
};

/*!
  \class ContainsPlayerPredicate
  \brief check if target player is in region
*/
template< typename T >
class ContainsPlayerPredicate
    : public PlayerPredicate {
private:
    //! checked geomotry
    T M_region;

public:

    /*!
      \brief construct with the geometry object
      \param geom geometry object for checking
    */
    ContainsPlayerPredicate( const T & region )
        : M_region( region )
      { }

    /*!
      \brief predicate function
      \param p const reference to the target player object
      \return true if target player is in the region
    */
    bool operator()( const AbstractPlayerObject & p ) const
      {
          return M_region.contains( p.pos() );
      }

    /*!
      \brief create clone object.
      \return cloned object.
     */
    PlayerPredicate::Ptr clone() const
      {
          return Ptr( new ContainsPlayerPredicate( M_region ) );
      }
};

}

#endif
