// -*-c++-*-

/*!
  \file time_stamp.h
  \brief time info Header File
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

#ifndef RCSC_TIME_TIME_STAMP_H
#define RCSC_TIME_TIME_STAMP_H

#include <chrono>
#include <cstdint>

namespace rcsc {

/*!
  \class TimeStamp
  \brief wrapper class for the system time point
*/
class TimeStamp {
public:
    using value_type = std::chrono::system_clock::time_point;

private:
    value_type M_time_point;

public:

    /*!
      \brief construct an invalid time stamp instance. time_point is initialized with duration::zero()
    */
    TimeStamp()
        : M_time_point()
    { }

    /*!
      \brief construct with the given time point
      \param tp time point
    */
    explicit
    TimeStamp( const value_type & tp )
        : M_time_point( tp )
    { }

    bool isValid() const
    {
        return M_time_point.time_since_epoch().count() > 0;
    }

    /*!
      \brief update to the current time point
    */
    void setNow()
    {
        M_time_point = std::chrono::system_clock::now();
    }

    /*!
      \brief get the time point value
      \return const reference to the time_point instance
    */
    const value_type & timePoint() const
    {
        return M_time_point;
    }

    /*!
      \brief get the milliseconds value since the given time stamp
      \return count value in the order of millisecond
    */
    std::int64_t elapsedSince( const TimeStamp & other ) const
    {
        return std::chrono::duration_cast< std::chrono::milliseconds >( this->timePoint() - other.timePoint() ).count();
    }

};

/*-------------------------------------------------------------------*/
/*!
  \brief operator '<' for rcsc::TimeStamp
  \param lhs left hand side argument
  \param rhs right hand side argument
  \return boolean value
*/
inline
bool
operator<( const rcsc::TimeStamp & lhs,
           const rcsc::TimeStamp & rhs )
{
    return lhs.timePoint() < rhs.timePoint();
}

/*-------------------------------------------------------------------*/
/*!
  \brief operator '>' for rcsc::TimeStamp
  \param lhs left hand side argument
  \param rhs right hand side argument
  \return boolean value
*/
inline
bool
operator>( const rcsc::TimeStamp & lhs,
           const rcsc::TimeStamp & rhs )
{
    return lhs.timePoint() > rhs.timePoint();
}

}

#endif
