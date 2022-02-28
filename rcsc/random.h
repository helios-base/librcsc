// -*-c++-*-

/*!
  \file random.h
  \brief random utility Header File
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

#ifndef RCSC_UTIL_RANDOM_H
#define RCSC_UTIL_RANDOM_H

#include <random>
#include <algorithm> // min, max
#include <iostream>
#include <ctime>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!
  \class RandomEngine
  \brief random engine holder for boost.
*/
class RandomEngine {
public:
    //! alias of the randome engine object type.
    typedef std::mt19937 base_type;
private:
    //! engine object
    base_type M_engine;

    /*!
      \brief default constructor

      default constructor is private, because this class is singleton.
      The random engine object is seeded by current time.
    */
    RandomEngine()
        : M_engine( std::random_device()() )
      { }

    //! not used for singleton.
    RandomEngine( const RandomEngine & ) = delete;
    //! not used for singleton.
    RandomEngine & operator=( const RandomEngine & ) = delete;

public:

    /*!
      \brief singleton interface.
      \return reference to the instance.
    */
    static
    RandomEngine & instance()
      {
          static RandomEngine s_instance;
          return s_instance;
      }

    /*!
      \brief apply new seed to random engine.
      \param value seed value
    */
    void seed( base_type::result_type value )
      {
          M_engine.seed( value );
      }

    /*!
      \brief get engine object
      \return reference to the random engine
    */
    base_type & get()
      {
          return M_engine;
      }
};

/*-------------------------------------------------------------------*/
/*!
  \class UniformRNG
  \brief template uniform random number generator class.
  DstType must be uniform type.
*/
template < typename DstType >
class UniformRNG {
public:
    //! alias of the result value type
    typedef typename DstType::result_type result_type;

private:
    //! random number generator object
    DstType M_dst;

    //! default constructor must not be used
    UniformRNG();
public:
    /*!
      \brief construct with value range
      \param min minimum value
      \param max maximum value
     */
    UniformRNG( result_type min,
                result_type max )
        : M_dst( min, max )
      { }

    /*!
      \brief functional operator
      \return generated random value with range
     */
    result_type operator()()
      {
          return M_dst( RandomEngine::instance().get() );
      }
};

//! alias of the uniform int number generator
typedef UniformRNG< std::uniform_int_distribution<> > UniformInt;
//! alias of the uniform real number generator
typedef UniformRNG< std::uniform_real_distribution<> > UniformReal;

}

#endif
