// -*-c++-*-

/*!
  \file timer.h
  \brief milli second timer Header File
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

#ifndef RCSC_TIME_TIMER_H
#define RCSC_TIME_TIMER_H

#include <rcsc/time/time_stamp.h>

#include <chrono>
#include <cstdint>

namespace rcsc {

/*!
  \class Timer
  \brief this class enables to measure the elapsed time.
*/
class Timer {
public:
    /*!
      \enum Type
      \brief unit type of returned value
    */
    enum Type {
        MSec, //!< milli second
        Sec, //!< second
        Min, //!< minute
        Hour, //!< hour
        Day, //!< day
    };

private:
    //! started time
    TimeStamp M_start_time;

public:
    /*!
      \brief construct with the current system clock time
    */
    Timer()
        : M_start_time( std::chrono::system_clock::now() )
    { }

    /*!
      \brief reset the start time
    */
    void restart()
    {
        M_start_time.setNow();
    }

    /*!
      \brief elapsed milli seconds since last start time.
      \return elapsed milli seconde by long integer
    */
    std::int64_t elapsed( const Type type = MSec ) const;

    /*!
      \brief elapsed milli seconds since last start time.
      \return elapsed milli seconde by floating point number
    */
    double elapsedReal( const Type type = MSec ) const;
};

}

#endif
