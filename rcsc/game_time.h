// -*-c++-*-

/*!
  \file game_time.h
  \brief game time depending on RCSSServer2D Header File
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

#ifndef RCSC_GAME_TIME_H
#define RCSC_GAME_TIME_H

#include <iostream>

namespace rcsc {

/*!
  \class GameTime
  \brief game time object
 */
class GameTime {
private:
    //! normal simulation time
    long M_cycle;
    //! stoppage time
    long M_stopped;

public:
    /*!
      \brief default constructor. the values are set to 0.
    */
    GameTime()
        : M_cycle( 0 ),
          M_stopped( 0 )
      { }

    /*!
      \brief construct with the specified values.
      \param c normal cycle count
      \param s stopped cycle count
     */
    GameTime( const long c,
              const long s )
        : M_cycle( c ),
          M_stopped( s )
      { }

    /*!
      \brief get normal time value
      \return const reference to the normal cycle value
     */
    long cycle() const
      {
          return M_cycle;
      }

    /*!
      \brief get stoppage time value
      \return const reference to the stopped cycle value
     */
    long stopped() const
      {
          return M_stopped;
      }

    /*!
      \brief assign new value
      \param c new normal cycle value
      \param s new stopped cycle value
      \return const reference to itself
     */
    const GameTime & assign( const long c,
                             const long s )
      {
          M_cycle = c;
          M_stopped = s;
          return *this;
      }

    /*!
      \brief assign new normal cycle
      \param c new normal cycle value
      \return const reference to itself
     */
    const GameTime & setCycle( const long c )
      {
          M_cycle = c;
          return *this;
      }

    /*!
      \brief assign new stopped cycle
      \param s new stopped cycle value
      \return const reference to itself
     */
    const GameTime & setStopped( const long s )
      {
          M_stopped = s;
          return *this;
      }

    /*!
      \brief add to normal cycle
      \param t added value
      \return const reference to itself
     */
    const GameTime & addCycle( const long t )
      {
          M_cycle += t;
          return *this;
      }

    /*!
      \brief add to stopped cycle
      \param t added value
      \return const reference to itself
     */
    const GameTime & addStopped( const long t )
      {
          M_stopped += t;
          return *this;
      }

    /*!
      \struct Less
      \brief compare function
     */
    struct Less {
        bool operator()( const GameTime & lhs,
                         const GameTime & rhs ) const
          {
              return ( lhs.cycle() < rhs.cycle()
                       || ( lhs.cycle() == rhs.cycle()
                            && lhs.stopped() < rhs.stopped() )
                       );
          }
    };

    /*!
      \struct Greater
      \brief compare function
     */
    struct Greater {
        bool operator()( const GameTime & lhs,
                         const GameTime & rhs ) const
          {
              return ( lhs.cycle() > rhs.cycle()
                       || ( lhs.cycle() == rhs.cycle()
                            && lhs.stopped() > rhs.stopped() )
                       );
          }
    };
};

} // end namespace

/*-------------------------------------------------------------------*/
/*!
  \brief output stream operator
  \param o reference to the output stream
  \param t const reference to the GameTime object
  \return reference to the output stream
*/
inline
std::ostream &
operator<<( std::ostream & o,
            const rcsc::GameTime & t )
{
    o << "[" << t.cycle() << ", " << t.stopped() << "]";
    return o;
}

/*-------------------------------------------------------------------*/
/*!
  \brief operator '==' for rcsc::GameTime
  \param lhs left hand side argument
  \param rhs right hand side argument
  \retval true all values are same
  \retval false some values are not same
*/
inline
bool
operator==( const rcsc::GameTime & lhs,
            const rcsc::GameTime & rhs )
{
    return ( lhs.cycle() == rhs.cycle()
             && lhs.stopped() == rhs.stopped() );
}

/*-------------------------------------------------------------------*/
/*!
  \brief operator '!=' for rcsc::GameTime
  \param lhs left hand side argument
  \param rhs right hand side argument
  \return boolean value
*/
inline
bool operator!=( const rcsc::GameTime & lhs,
                 const rcsc::GameTime & rhs )
{
    return !( lhs == rhs );
}

/*-------------------------------------------------------------------*/
/*!
  \brief operator '<' for rcsc::GameTime
  \param lhs left hand side argument
  \param rhs right hand side argument
  \return boolean value
*/
inline
bool
operator<( const rcsc::GameTime & lhs,
           const rcsc::GameTime & rhs )
{
    return ( lhs.cycle() < rhs.cycle()
             || ( lhs.cycle() == rhs.cycle()
                  && lhs.stopped() < rhs.stopped() )
             );
}

/*-------------------------------------------------------------------*/
/*!
  \brief operator '<=' for rcsc::GameTime
  \param lhs left hand side argument
  \param rhs right hand side argument
  \return boolean value
*/
inline
bool
operator<=( const rcsc::GameTime & lhs,
            const rcsc::GameTime & rhs )
{
    return ( lhs.cycle() < rhs.cycle()
             || ( lhs.cycle() == rhs.cycle()
                  && lhs.stopped() <= rhs.stopped() )
             );
}

/*-------------------------------------------------------------------*/
/*!
  \brief operator '>' for rcsc::GameTime
  \param lhs left hand side argument
  \param rhs right hand side argument
  \return boolean value
*/
inline
bool
operator>( const rcsc::GameTime & lhs,
           const rcsc::GameTime & rhs )
{
    return ( lhs.cycle() > rhs.cycle()
             || ( lhs.cycle() == rhs.cycle()
                  && lhs.stopped() > rhs.stopped() )
             );
}

/*-------------------------------------------------------------------*/
/*!
  \brief operator '>=' for rcsc::GameTime
  \param lhs left hand side argument
  \param rhs right hand side argument
  \return boolean value
*/
inline
bool
operator>=(  const rcsc::GameTime & lhs,
             const rcsc::GameTime & rhs )
{
    return ( lhs.cycle() > rhs.cycle()
             || ( lhs.cycle() == rhs.cycle()
                  && lhs.stopped() >= rhs.stopped() )
             );
}

#endif
