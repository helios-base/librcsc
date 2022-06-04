// -*-c++-*-

/*!
  \file timer.cpp
  \brief milli second timer Source File
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <rcsc/timer.h>

#include <iostream>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
std::int64_t
Timer::elapsed( const Type type ) const
{
    switch ( type ) {
    case MSec:
        return std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::system_clock::now() - M_start_time.timePoint() ).count();
    case Sec:
        return std::chrono::duration_cast< std::chrono::seconds >( std::chrono::system_clock::now() - M_start_time.timePoint() ).count();
    case Min:
        return std::chrono::duration_cast< std::chrono::minutes >( std::chrono::system_clock::now() - M_start_time.timePoint() ).count();
    case Hour:
        return std::chrono::duration_cast< std::chrono::hours >( std::chrono::system_clock::now() - M_start_time.timePoint() ).count();
    case Day:
        return std::chrono::duration_cast< std::chrono::hours >( std::chrono::system_clock::now() - M_start_time.timePoint() ).count() / 24;
    default:
        break;
    }

    return 0;
}

/*-------------------------------------------------------------------*/
/*!

*/
double
Timer::elapsedReal( const Type type ) const
{
    const double nano = std::chrono::duration_cast< std::chrono::nanoseconds >( std::chrono::system_clock::now() - M_start_time.timePoint() ).count();

    switch ( type ) {
    case MSec:
        return nano * 0.001 * 0.001;
    case Sec:
        return nano * 0.001 * 0.001 * 0.001;
    case Min:
        return nano * 0.001 * 0.001 * 0.001 / 60.0;
    case Hour:
        return nano * 0.001 * 0.001 * 0.001 * 0.001 / 60.0 / 60.0;
    case Day:
        return nano * 0.001 * 0.001 * 0.001 * 0.001 / 60.0 / 60.0 / 24.0;
    default:
        break;
    }

    return 0.0;
}

}
