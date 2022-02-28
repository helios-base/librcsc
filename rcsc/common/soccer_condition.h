// -*-c++-*-

/*!
  \file soccer_condition.h
  \brief abstract condition class Header File
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

#ifndef RCSC_PLAYER_SOCCER_CONDITION_H
#define RCSC_PLAYER_SOCCER_CONDITION_H

#include <memory>
#include <vector>

namespace rcsc {

class PlayerAgent;

/*!
  \class Condition
  \brief abstract condition
*/
class Condition {
private:

    //! not used
    Condition( const Condition & ) = delete;
    //! not used
    Condition & operator=( const Condition & ) = delete;

protected:
    /*!
      \brief nothing to do. but accessible only from derived
      classes.
     */
    Condition() = default;

public:
    /*!
      \brief nothing to do, but should be virtual.
     */
    virtual
    ~Condition() = default;

    /*!
      \brief get the value of this condition.
      \return the value of this condition.
     */
    virtual
    bool operator()( const PlayerAgent * agent ) const = 0;
};

/*!
  \class AndCondition
  \brief composite "logical and" condition
 */
class AndCondition
    : public Condition {
private:
    //! container of condition object
    std::vector< std::shared_ptr< const Condition > > M_condition_set;

public:

    /*!
      \brief construct with 2 conditions
     */
    AndCondition( const std::shared_ptr< const Condition > & p1,
                  const std::shared_ptr< const Condition > & p2 );

    /*!
      \brief construct with 3 conditions
     */
    AndCondition( const std::shared_ptr< const Condition > & p1,
                  const std::shared_ptr< const Condition > & p2,
                  const std::shared_ptr< const Condition > & p3 );

    /*!
      \brief construct with 4 conditions
     */
    AndCondition( const std::shared_ptr< const Condition > & p1,
                  const std::shared_ptr< const Condition > & p2,
                  const std::shared_ptr< const Condition > & p3,
                  const std::shared_ptr< const Condition > & p4 );

    /*!
      \brief construct with 5 conditions
     */
    AndCondition( const std::shared_ptr< const Condition > & p1,
                  const std::shared_ptr< const Condition > & p2,
                  const std::shared_ptr< const Condition > & p3,
                  const std::shared_ptr< const Condition > & p4,
                  const std::shared_ptr< const Condition > & p5 );

    /*!
      \brief construct with 6 conditions
     */
    AndCondition( const std::shared_ptr< const Condition > & p1,
                  const std::shared_ptr< const Condition > & p2,
                  const std::shared_ptr< const Condition > & p3,
                  const std::shared_ptr< const Condition > & p4,
                  const std::shared_ptr< const Condition > & p5,
                  const std::shared_ptr< const Condition > & p6 );

    /*!
      \brief construct with 7 conditions
     */
    AndCondition( const std::shared_ptr< const Condition > & p1,
                  const std::shared_ptr< const Condition > & p2,
                  const std::shared_ptr< const Condition > & p3,
                  const std::shared_ptr< const Condition > & p4,
                  const std::shared_ptr< const Condition > & p5,
                  const std::shared_ptr< const Condition > & p6,
                  const std::shared_ptr< const Condition > & p7 );

    /*!
      \brief construct with 8 conditions
     */
    AndCondition( const std::shared_ptr< const Condition > & p1,
                  const std::shared_ptr< const Condition > & p2,
                  const std::shared_ptr< const Condition > & p3,
                  const std::shared_ptr< const Condition > & p4,
                  const std::shared_ptr< const Condition > & p5,
                  const std::shared_ptr< const Condition > & p6,
                  const std::shared_ptr< const Condition > & p7,
                  const std::shared_ptr< const Condition > & p8 );

    /*!
      \brief append new condition.
     */
    void append( const std::shared_ptr< const Condition > & p )
      {
          M_condition_set.push_back( p );
      }

    /*!
      \brief get the value of the 'logical and'
      \return the value of the condition set.
     */
    bool operator()( const PlayerAgent * agent ) const;

};


/*!
  \class OrCondition
  \brief composite "logical or" condition
 */
class OrCondition
    : public Condition {
private:
    //! container of condition object
    std::vector< std::shared_ptr< const Condition > > M_condition_set;

public:

    /*!
      \brief construct with 2 conditions
     */
    OrCondition( const std::shared_ptr< const Condition > & p1,
                 const std::shared_ptr< const Condition > & p2 );

    /*!
      \brief construct with 3 conditions
     */
    OrCondition( const std::shared_ptr< const Condition > & p1,
                 const std::shared_ptr< const Condition > & p2,
                 const std::shared_ptr< const Condition > & p3 );

    /*!
      \brief construct with 4 conditions
     */
    OrCondition( const std::shared_ptr< const Condition > & p1,
                 const std::shared_ptr< const Condition > & p2,
                 const std::shared_ptr< const Condition > & p3,
                 const std::shared_ptr< const Condition > & p4 );

    /*!
      \brief construct with 5 conditions
     */
    OrCondition( const std::shared_ptr< const Condition > & p1,
                 const std::shared_ptr< const Condition > & p2,
                 const std::shared_ptr< const Condition > & p3,
                 const std::shared_ptr< const Condition > & p4,
                 const std::shared_ptr< const Condition > & p5 );

    /*!
      \brief construct with 6 conditions
     */
    OrCondition( const std::shared_ptr< const Condition > & p1,
                 const std::shared_ptr< const Condition > & p2,
                 const std::shared_ptr< const Condition > & p3,
                 const std::shared_ptr< const Condition > & p4,
                 const std::shared_ptr< const Condition > & p5,
                 const std::shared_ptr< const Condition > & p6 );

    /*!
      \brief construct with 7 conditions
     */
    OrCondition( const std::shared_ptr< const Condition > & p1,
                 const std::shared_ptr< const Condition > & p2,
                 const std::shared_ptr< const Condition > & p3,
                 const std::shared_ptr< const Condition > & p4,
                 const std::shared_ptr< const Condition > & p5,
                 const std::shared_ptr< const Condition > & p6,
                 const std::shared_ptr< const Condition > & p7 );

    /*!
      \brief construct with 8 conditions
     */
    OrCondition( const std::shared_ptr< const Condition > & p1,
                 const std::shared_ptr< const Condition > & p2,
                 const std::shared_ptr< const Condition > & p3,
                 const std::shared_ptr< const Condition > & p4,
                 const std::shared_ptr< const Condition > & p5,
                 const std::shared_ptr< const Condition > & p6,
                 const std::shared_ptr< const Condition > & p7,
                 const std::shared_ptr< const Condition > & p8 );

    /*!
      \brief append new condition.
     */
    void append( const std::shared_ptr< const Condition > & p )
      {
          M_condition_set.push_back( p );
      }

    /*!
      \brief get the value of the 'logical or'
      \return the value of the condition set.
     */
    bool operator()( const PlayerAgent * agent ) const;

};


/*!
  \class NotCondition
  \brief negate condition
 */
class NotCondition {
private:

    //! target condition
    std::shared_ptr< const Condition > M_condition;

public:

    /*!
      \brief construct with target condition
     */
    NotCondition( const std::shared_ptr< const Condition >  & p )
        : M_condition( p )
      { }

    /*!
      \brief get the value of the 'logical not'
      \return the value of the condition set.
     */
    bool operator()( const PlayerAgent * agent ) const
      {
          return ( M_condition
                   && ! (*M_condition)( agent ) );
      }

};

}

#endif
