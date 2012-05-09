// -*-c++-*-

/*!
  \file angle_deg.cpp
  \brief Degree wrapper class Source File.
*/

/*
 *Copyright:

 Copyright (C) 2004 Hidehisa Akiyama

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

#include "angle_deg.h"

#include <algorithm>
#include <vector>

#ifndef M_PI
//! PI value macro
#define M_PI 3.14159265358979323846
#endif

namespace rcsc {

const double AngleDeg::EPSILON = 1.0e-5;

const double AngleDeg::PI = M_PI;
const double AngleDeg::TWO_PI = M_PI * 2.0;
const double AngleDeg::DEG2RAD = M_PI / 180.0;
const double AngleDeg::RAD2DEG = 180.0 / M_PI;

/*-------------------------------------------------------------------*/
/*!

 */
bool
AngleDeg::isWithin( const AngleDeg & left,
                    const AngleDeg & right ) const
{
    // left to right arc angle is less than 180 degree.
    if ( left.isLeftEqualOf( right ) )
    {
        if ( left.isLeftEqualOf( *this ) && this->isLeftEqualOf( right ) )
        {
            return true;
        }
    }
    // arc angle is more than 180 degree.
    else
    {
        // check out reverse side
        //if ( *this <= right || left <= *this )
        // == !(right < *this && *this < left)
        if ( this->isLeftEqualOf( right ) || left.isLeftEqualOf( *this ) )
        {
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
AngleDeg::sinMinMax( const double angle_err,
                     double * minsin,
                     double * maxsin ) const
{
    if ( angle_err < 0.0 || 180.0 < angle_err )
    {
        std::cerr << "AngleDeg::sinMinMax() invalid error range. "
                  << angle_err << std::endl;
        *minsin = -1.0;
        *maxsin = 1.0;
        return;
    }

    double mindir = this->degree() - angle_err;
    double maxdir = this->degree() + angle_err;

    std::vector< double > sol;
    sol.reserve( 4 );

    if ( ( mindir < -90.0 && -90.0 < maxdir )
         || ( mindir < 270.0 && 270.0 < maxdir )
         )
    {
        sol.push_back( -1.0 );
    }

    if ( ( mindir < 90.0 && 90.0 < maxdir )
         || ( mindir < -270.0 && -270.0 < maxdir )
         )
    {
        sol.push_back( 1.0 );
    }

    sol.push_back( AngleDeg::sin_deg( mindir ) );
    sol.push_back( AngleDeg::sin_deg( maxdir ) );

    *minsin = *std::min_element( sol.begin(), sol.end() );
    *maxsin = *std::max_element( sol.begin(), sol.end() );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AngleDeg::cosMinMax( const double angle_err,
                     double * mincos,
                     double * maxcos ) const
{
    if ( angle_err < 0.0 || 180.0 < angle_err )
    {
        std::cerr << "AngleDeg::cosMinMax() invalid error range. "
                  << angle_err << std::endl;
        *mincos = -1.0;
        *maxcos = 1.0;
        return;
    }

    double mindir = this->degree() - angle_err;
    double maxdir = this->degree() + angle_err;

    std::vector< double > sol;
    sol.reserve( 4 );

    if ( mindir < -180.0 && -180.0 < maxdir )
    {
        sol.push_back( -1.0 );
    }

    if ( mindir < 0.0 && 0.0 < maxdir )
    {
        sol.push_back( 1.0 );
    }

    sol.push_back( AngleDeg::cos_deg( mindir ) );
    sol.push_back( AngleDeg::cos_deg( maxdir ) );

    *mincos = *std::min_element( sol.begin(), sol.end() );
    *maxcos = *std::max_element( sol.begin(), sol.end() );
}


/*-------------------------------------------------------------------*/
/*!

 */
AngleDeg
AngleDeg::bisect( const AngleDeg & left,
                  const AngleDeg & right )
{
    AngleDeg result( left );
    AngleDeg rel( right - left );
    double half_deg = rel.degree() * 0.5;
    result += half_deg;

    if ( left.isLeftOf( right ) )
    {
        return result;
    }
    else
    {
        return result += 180.0;
    }
}

} // end of namespace
