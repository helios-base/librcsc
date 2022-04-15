// -*-c++-*-

/*!
  \file math_util.h
  \brief generic math utility Header File.
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

#ifndef RCSC_MATH_UTIL_H
#define RCSC_MATH_UTIL_H

#include <algorithm>
#include <cmath>

namespace rcsc {

//! epsilon value
constexpr double EPS = 1.0e-10;

/*-------------------------------------------------------------------*/
/*!
  \brief bound value within the range [low, high]
  \param low low side value
  \param x input value
  \param high high side value
  \return normalized value
*/
template < typename  T >
const T &
bound( const T & low, const T & x, const T & high )
{
    return std::min( std::max( low, x ), high );
}

/*-------------------------------------------------------------------*/
/*!
  \brief bound value within the range [low, high] (same as rcsc::bound())
  \param low low side value
  \param x input value
  \param high high side value
  \return normalized value
*/
template < typename  T >
const T &
min_max( const T & low, const T & x, const T & high )
{
    return std::min( std::max( low, x ), high );
}

/*-------------------------------------------------------------------*/
/*!
  \brief calculate squared value.
  \param x value to be squared
*/
template < typename T >
T
square( const T & x )
{
    return x * x;
}

/*-------------------------------------------------------------------*/
/*!
  \brief get a sign value
  \param x value to be checked
  \retval 1.0 if x > 0.0
  \retval -1.0 if x <= 0.0
*/
inline
double
sign( const double & x )
{
    return x > 0.0 ? 1.0 : -1.0;
}

/*-------------------------------------------------------------------*/
/*!
  \brief round a floating point number with specified precision
  \param value value to be rounded
  \param prec round precision
  \return rounded value
*/
inline
double
round( const double & value,
       const double & prec )
{
    return rint( value / prec ) * prec;
}

/*-------------------------------------------------------------------*/
/*!
  \brief Quadratic Formulation (ax^2 + bx + c = 0)
  \param a coefficiant A
  \param b coefficiant B
  \param c constant C
  \param sol1 reference to the solution variable
  \param sol2 reference to the solution variable
  \return the number of solutions.
*/
inline
int
quadratic_formula( const double & a,
                   const double & b,
                   const double & c,
                   double * sol1,
                   double * sol2 )
{
    double d = b * b - 4.0 * a * c;

    // ignore small noise
    if ( std::fabs( d ) < 0.001 )
    {
        if ( sol1 ) *sol1 = -b / ( 2.0 * a );
        return 1;
    }

    if ( d < 0.0 )
    {
        return 0;
    }

    d = std::sqrt( d );
    if ( sol1 ) *sol1 = (-b + d) / (2.0 * a);
    if ( sol2 ) *sol2 = (-b - d) / (2.0 * a);
    return 2;
}

/*-------------------------------------------------------------------*/
/*!
  \brief calculate the sum of a geometric series
  \param first_term the value of first term
  \param r multiplication ratio
  \param len length of geometric series
  \return sum of geometric series

  (TOUHI SUURETSU NO WA)
  *** 0 < len ***
*/
inline
double
calc_sum_geom_series( const double & first_term,
                      const double & r,
                      const int len )
{
    // sum     = f + fr + fr^2 + ... + fr^(n-1)
    // sum * r =     fr + fr^2 + ... + fr^(n-1) + fr^n
    // sum * ( r - 1 ) = fr^n - f
    // sum = f * ( r^n - 1.0 ) / ( r - 1 )
    return first_term * ( ( std::pow( r, len ) - 1.0 ) / ( r - 1.0 ) );
}


/*-------------------------------------------------------------------*/
/*!
  \brief the sum of a infinity geometric series
  \param first_term the value of first term
  \param r rate
  \return sum of the geometric series

 (MUGEN TOUHI SUURETSU NO WA)
 *** 0.0 < r < 1.0 ***
*/
inline
double
calc_sum_inf_geom_series( const double & first_term,
                          const double & r )
{
    if ( r < 0.0 || 1.0 <= r )
    {
        return 0.0;
    }

    // limit(n->inf, 0<r<1)  sum = f * ( 1 - r^n ) / ( 1 - r )
    return first_term / ( 1.0 - r );
}

/*-------------------------------------------------------------------*/
/*!
  \brief caluculate the first term value of a geometric series.
  \param sum the sum of geometric series that length is n
  \param r multiplication ratio
  \param len length of geometric series
  \return the value of first term

  (TOUHI SUURETSU NO 1KOU)
*/
inline
double
calc_first_term_geom_series( const double & sum,
                             const double & r,
                             const int len )
{
    // sum = f * ( 1 - r^n ) / ( 1 - r )
    // f   = sum * ( 1 - r ) / ( 1 - r^n )
    return sum * ( 1.0 - r ) / ( 1.0 - std::pow( r, len ) );
}

/*-------------------------------------------------------------------*/
/*!
  \brief caluculate the first term value of a infinity geometric series.
  \param sum sum of infinite geometric series
  \param r multiplication ratio (should be less than 1.0)
  \return the value of first term

  (MUGEN TOUHI SUURETSU NO DAI 1 KOU)
  *** 0 < r < 1 ***
  e.g. use to get the ball's first speed when ball speed at dest is ZERO.
*/
inline
double
calc_first_term_inf_geom_series( const double & sum,
                                 const double & r )
{
    // limit(n->inf, 0<r<1) f = sum * ( 1 - r ) / ( 1 - r^n )
    return sum * ( 1.0 - r );
}

/*-------------------------------------------------------------------*/
/*!
  \brief caluculate the first term value of a geometric series.
  \param last_term value of the last term
  \param sum sum of infinite geometric series
  \param r multiplication ratio (should be less than 1.0)
  \return the value of first term

  (TOUHI SUURETSU NO DAI 1 KOU)
  *** 0 < r < 1 ***
  e.g. use to get the ball's first speed when ball speed at dest is ZERO.
*/
inline
double
calc_first_term_geom_series_last( const double & last_term,
                                  const double & sum,
                                  const double & r )
{
    if ( std::fabs( last_term ) < 0.001 )
    {
        return sum * ( 1.0 - r );
    }

    // l + (l * 1/r) + ... + (l * 1/r^(n-1))               = sum
    //     (l * 1/r) + ... + (l * 1/r^(n-1)) + (l * 1/r^n) = sum * (1/r)
    // l*(1/r^n) - l = sum * (1/r - 1)
    // (1/r^n) = sum * (1/r - 1) / l + 1
    double inverse = 1.0 / r;
    double tmp = 1.0 + sum * (inverse - 1.0) / last_term;
    if ( tmp < 0.001 )
    {
        return last_term;
    }

    //double len = std::log( tmp ) / std::log( inverse );
    //return last_term * std::pow( inverse, len );
    return last_term * std::pow( inverse, std::log( tmp ) / std::log( inverse ) );
}

/*-------------------------------------------------------------------*/
/*!
  \brief caluculate the length of a geometric series
  \param first_term value of the first term
  \param r multiplication ratio
  \param sum sum of a geometric series
  \return a round number of the length of geometric series

  (TOUHI SUURETSU NO NAGASA)
*/
inline
double
calc_length_geom_series( const double & first_term,
                         const double & sum,
                         const double & r )
{
    if ( first_term <= EPS
         || sum < 0.0
         || r <= EPS )
    {
        // cannot take the zero first term
        // cannot take the negative sum
        // cannot take the negative ratio
        return -1.0;
    }

    if ( sum <= EPS )
    {
        // already there
        return 0.0;
    }

    // f + fr + fr^2 + ... + fr^(n-1)        = sum
    //     fr + fr^2 + ... + fr^(n-1) + fr^n = sum * r
    // fr^n - f = sum * ( r - 1 )
    // r^n = 1 + sum * ( r - 1 ) / f

    double tmp = 1.0 + sum * ( r - 1.0 ) / first_term;
    if ( tmp <= EPS )
    {
        return -1.0;
    }
    return std::log( tmp ) / std::log( r );
}

} // end namespace

#endif
