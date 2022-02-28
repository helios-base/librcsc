// -*-c++-*-

/*!
  \file angle_deg.h
  \brief degree wrapper class Header File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa Akiyama

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

#ifndef RCSC_GEOM_ANGLEDEG_H
#define RCSC_GEOM_ANGLEDEG_H

#include <functional>
#include <iostream>
#include <cmath>

namespace rcsc {

/*!
  \class AngleDeg
  \brief degree wrapper class
*/
class AngleDeg {
    // : public boost::addable< AngleDeg >
    // , public boost::addable2< AngleDeg, double >
    // , public boost::addable2_left< double,  AngleDeg >
    // , public boost::subtractable< AngleDeg >
    // , public boost::subtractable2< AngleDeg, double >
    // , public boost::subtractable2_left< double, AngleDeg >
    // , public boost::multipliable2< AngleDeg, double >
    // , public boost::dividable2< AngleDeg, double >
private:
    //! degree, NOT radian
    double M_degree;

public:
    //! epsilon value
    static const double EPSILON;

    //! pi valur
    static const double PI;
    //! 2*pi valur
    static const double TWO_PI;
    //! constant variable to convert DEGREE to RADIAN.
    static const double DEG2RAD;
    //! constant variable to convert RADIAN to DEGREE.
    static const double RAD2DEG;

    //! default constructor.
    AngleDeg()
        : M_degree( 0.0 )
      { }
    /*!
      \brief constructor with value.

      NO explicit.
      This means that we can use this class as follows.
      AngleDeg angle = 3.0;
    */
    AngleDeg( const double deg )
        : M_degree( deg )
      {
          normalize();
      }

    /*!
      \brief operator substitution.
      \param deg initialization parameter
      \return const reference to itself
    */
    const AngleDeg & operator=( const double deg )
      {
          M_degree = deg;
          return normalize();
      }

private:
    /*!
      \brief normalize the value to [-180, 180].
      \return const reference to itself.
    */
    const AngleDeg & normalize()
      {
          if ( M_degree < -360.0 || 360.0 < M_degree )
          {
              M_degree = std::fmod( M_degree, 360.0 );
          }

          if ( M_degree < -180.0 )
          {
              M_degree += 360.0;
          }

          if ( M_degree > 180.0 )
          {
              M_degree -= 360.0;
          }

          return *this;
      }

public:

    /*!
      \brief get value of this angle
      \return const reference to the member variable
     */
    double degree() const
      {
          return M_degree;
      }

    /*!
      \brief get absolute value of this angle
      \return absolute value of degree()
     */
    double abs() const
      {
          return std::fabs( degree() );
      }
    /*!
      \brief get RADIAN value.
      \return radian value of degree()
     */
    double radian() const
      {
          return degree() * DEG2RAD;
      }

    /*
      operator double() const
      {
      return degree();
      }
    */

    /*!
      \brief get new AngleDeg multiplyed by -1.
      \return new AngleDeg object
     */
    AngleDeg operator-() const
      {
          return AngleDeg( - degree() );
      }

    /*!
      \brief operator += with AngleDeg
      \param angle added value
      \return const reference to this
    */
    const AngleDeg & operator+=( const AngleDeg & angle )
      {
          M_degree += angle.degree();
          return normalize();
      }

    /*!
      \brief operator += with double
      \param deg added value
      \return const reference to this
    */
    const AngleDeg & operator+=( const double deg )
      {
          M_degree += deg;
          return normalize();
      }

    /*!
      \brief operator -= with AngleDeg
      \param angle subtract argument
      \return const reference to this
     */
    const AngleDeg & operator-=( const AngleDeg & angle )
      {
          M_degree -= angle.degree();
          return normalize();
      }

    /*!
      \brief operator -= with double
      \param deg subtract argument
      \return const reference to this
     */
    const AngleDeg & operator-=( const double deg )
      {
          M_degree -= deg;
          return normalize();
      }

    /*!
      \brief operator *=
      \param scalar multiply argument
      \return const reference to this
     */
    const AngleDeg & operator*=( const double scalar )
      {
          M_degree *= scalar;
          return normalize();
      }

    /*!
      \brief operator /=
      \param scalar division argument
      \return const reference to this
     */
    const AngleDeg & operator/=( const double scalar )
      {
          if ( std::fabs( scalar ) < EPSILON )
          {
              return * this;
          }
          M_degree /= scalar;
          return normalize();
      }

    //////////////////////////////////////////////////////////////////

    /*!
      \brief check if this angle is left of 'angle'
      \return true or false
     */
    bool isLeftOf( const AngleDeg & angle ) const
      {
          //return (*this - angle).degree() < 0.0;
          double diff = angle.degree() - this->degree();
          return ( ( 0.0 < diff && diff < 180.0 )
                   || diff < -180.0 );
      }

    /*!
      \brief check if this angle is left or equal of 'angle'
      \return true or false
     */
    bool isLeftEqualOf( const AngleDeg & angle ) const
      {
          //return (*this - angle).degree() <= 0.0;
          double diff = angle.degree() - this->degree();
          return ( ( 0.0 <= diff && diff < 180.0 )
                   || diff < -180.0 );
      }

    /*!
      \brief check if this angle is right of 'angle'
      \return true or false
     */
    bool isRightOf( const AngleDeg & angle ) const
      {
          //return (*this - angle).degree() > 0.0;
          double diff = this->degree() - angle.degree();
          return ( ( 0.0 < diff && diff < 180.0 )
                   || diff < -180.0 );
      }

    /*!
      \brief check if this angle is right or equal of 'angle'
      \return true or false
     */
    bool isRightEqualOf( const AngleDeg & angle ) const
      {
          //return (*this - angle).degree() >= 0.0;
          double diff = this->degree() - angle.degree();
          return ( ( 0.0 <= diff && diff < 180.0 )
                   || diff < -180.0 );
      }

    /*!
      \brief calculate cosine
      \return cosine value
     */
    double cos() const
      {
          return std::cos( degree() * DEG2RAD );
      }

    /*!
      \brief calculate sine
      \return sine value
     */
    double sin() const
      {
          return std::sin( degree() * DEG2RAD );
      }

    /*!
      \brief calculate tarngetn
      \return tangent value
     */
    double tan() const
      {
          return std::tan( degree() * DEG2RAD );
      }

    /*!
      \brief check if this angle is within [left, right] (turn clockwise).
      \param left left angle
      \param right right angle
      \return true if this is within [left, right]. else false
     */
    bool isWithin( const AngleDeg & left,
                   const AngleDeg & right ) const;

    /*!
      \brief calculate min/max sine value with angle error.
      \param angle_err error value of angle
      \param minsin pointer to the solution variable to store the min value
      \param maxsin pointer to the solution variable to store the max value
    */
    void sinMinMax( const double angle_err,
                    double * minsin,
                    double * maxsin ) const;

    /*!
      \brief calculate min/max cosine value of angle with angle error.
      \param angle_err error value of angle
      \param mincos pointer to the solution variable to store the min value
      \param maxcos pointer to the solution variable to store the max value
    */
    void cosMinMax( const double angle_err,
                    double * mincos,
                    double * maxcos ) const;

    ////////////////////////////////////////////////////////
    // static utility method concerned with angle operation

    /*!
      \brief static utility. normalize angle
      \param dir angle value to be normalized
      \return normalized angle value
     */
    inline
    static
    double normalize_angle( double dir )
      {
          if ( dir < -360.0 || 360.0 < dir )
          {
              dir = std::fmod( dir, 360.0 );
          }

          if ( dir < -180.0 )
          {
              dir += 360.0;
          }

          if ( dir > 180.0 )
          {
              dir -= 360.0;
          }

          return dir;
      }

    /*!
      \brief static utility. convert radian to degree
      \param rad radian value
      \return degree value
    */
    inline
    static
    double rad2deg( const double rad )
      {
          return rad * RAD2DEG;
      }

    /*!
      \brief static utility. convert degree to radian
      \param deg degree value
      \return radian value
    */
    inline
    static
    double deg2rad( const double deg )
      {
          return deg * DEG2RAD;
      }

    /*!
      \brief static utility. calculate cosine value for degree angle
      \param deg degree value
      \return cosine value
    */
    inline
    static
    double cos_deg( const double deg )
      {
          return std::cos( deg2rad( deg ) );
      }

    /*!
      \brief static utility. calculate sine value for degree angle
      \param deg degree value
      \return sine value
    */
    inline
    static
    double sin_deg( const double deg )
      {
          return std::sin( deg2rad( deg ) );
      }

    /*!
      \brief static utility. calculate tangent value for degree angle
      \param deg degree value
      \return tangent value
    */
    inline
    static
    double tan_deg( const double deg )
      {
          return std::tan( deg2rad( deg ) );
      }

    /*!
      \brief static utility. calculate arc cosine value
      \param cosine cosine value
      \return arc cosine value, that is degree type.
    */
    inline
    static
    double acos_deg( const double cosine )
      {
          return ( cosine >= 1.0
                   ? 0.0
                   : ( cosine <= -1.0
                       ? 180.0
                       : rad2deg( std::acos( cosine ) ) ) );
      }

    /*!
      \brief static utility. calculate arc sine value
      \param sine sine value
      \return arc sine value, that is degree type.
    */
    inline
    static
    double asin_deg( const double sine )
      {
          return ( sine >= 1.0
                   ? 90.0
                   : ( sine <= -1.0
                       ? -90.0
                       : rad2deg( std::asin( sine ) ) ) );
      }

    /*!
      \brief static utility. calculate arc tangent value
      \param tangent tangent value
      \return arc tangent value, that is degree.
    */
    inline
    static
    double atan_deg( const double tangent )
      {
          return rad2deg( std::atan( tangent ) );
      }

    /*!
      \brief static utility. calculate arc tangent value from XY
      \param y coordinate Y
      \param x coordinate X
      \return arc tangent value, that is degree type.
    */
    inline
    static
    double atan2_deg( const double y,
                      const double x )
      {
          return ( ( x == 0.0 && y == 0.0 )
                   ? 0.0
                   : rad2deg( std::atan2( y, x ) ) );
      }

    /*!
      \brief static utility that returns bisect angle of [left, right].
      \param left left start angle
      \param right right end angle
      \return bisect angle

      this methid can take obtuse angle.
    */
    static
    AngleDeg bisect( const AngleDeg & left,
                     const AngleDeg & right );

    ////////////////////////////////////////////////////////

    /*!
      \brief output value to ostream
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & print( std::ostream & os ) const
      {
          return os << degree();
      }

    /*!
      \brief output rounded value to ostream
      \param os reference to the output stream
      \param step round step
      \return reference to the output stream
     */
    std::ostream & printRound( std::ostream & os,
                               const double step = 0.1 ) const
      {
          return os << rint( degree() / step ) * step;
      }

};

} // end of namespace


////////////////////////////////////////////////////////
// arith operators

/*-------------------------------------------------------------------*/
/*!
  \brief operator add(T, T)
  \param lhs left hand side argument
  \param rhs right hand side argument
  \return new angle
 */
inline
const
rcsc::AngleDeg
operator+( const rcsc::AngleDeg & lhs,
           const rcsc::AngleDeg & rhs )
{
    return rcsc::AngleDeg( lhs ) += rhs;
}

/*-------------------------------------------------------------------*/
/*!
  \brief operator add(T, U)
  \param lhs left hand side argument
  \param rhs right hand side argument
  \return new angle
 */
inline
const
rcsc::AngleDeg
operator+( const rcsc::AngleDeg & lhs,
           const double rhs )
{
    return rcsc::AngleDeg( lhs ) += rhs;
}

/*-------------------------------------------------------------------*/
/*!
  \brief operator add(U, T)
  \param lhs left hand side argument
  \param rhs right hand side argument
  \return new angle
 */
inline
const
rcsc::AngleDeg
operator+( const double lhs,
           const rcsc::AngleDeg & rhs )
{
    return rcsc::AngleDeg( rhs ) += lhs;
}

/*-------------------------------------------------------------------*/
/*!
  \brief operator sub(T, T)
  \param lhs left hand side argument
  \param rhs right hand side argument
  \return new angle
 */
inline
const
rcsc::AngleDeg
operator-( const rcsc::AngleDeg & lhs,
           const rcsc::AngleDeg & rhs )
{
    return rcsc::AngleDeg( lhs ) -= rhs;
}

/*-------------------------------------------------------------------*/
/*!
  \brief operator sub(T, U)
  \param lhs left hand side argument
  \param rhs right hand side argument
  \return new angle
 */
inline
const
rcsc::AngleDeg
operator-( const rcsc::AngleDeg & lhs,
           const double rhs )
{
    return rcsc::AngleDeg( lhs ) -= rhs;
}

/*-------------------------------------------------------------------*/
/*!
  \brief operator sub(U, T)
  \param lhs left hand side argument
  \param rhs right hand side argument
  \return new angle
 */
inline
const
rcsc::AngleDeg
operator-( const double lhs,
           const rcsc::AngleDeg & rhs )
{
    return rcsc::AngleDeg( lhs ) -= rhs;
}

/////////////////////////////////////////////////////
// predicate operator

/*-------------------------------------------------------------------*/
/*!
  \brief predicate operator !=
  \param lhs left hand side argument
  \param rhs right hand side argument
  \return true or false
 */
inline
bool
operator!=( const rcsc::AngleDeg & lhs,
            const rcsc::AngleDeg & rhs )
{
    return std::fabs( lhs.degree() - rhs.degree() ) > rcsc::AngleDeg::EPSILON;
}

/*-------------------------------------------------------------------*/
/*!
  \brief predicate operator !=
  \param lhs left hand side argument
  \param rhs right hand side argument
  \return true or false
 */
inline
bool
operator!=( const rcsc::AngleDeg & lhs,
            const double rhs )
{
    return std::fabs( lhs.degree() - rhs ) > rcsc::AngleDeg::EPSILON;
}

/*-------------------------------------------------------------------*/
/*!
  \brief predicate operator !=
  \param lhs left hand side argument
  \param rhs right hand side argument
  \return true or false
 */
inline
bool
operator!=( const double lhs,
            const rcsc::AngleDeg & rhs )
{
    return std::fabs( lhs - rhs.degree() ) > rcsc::AngleDeg::EPSILON;
}

/*-------------------------------------------------------------------*/
/*!
  \brief predicate operator ==
  \param lhs left hand side argument
  \param rhs right hand side argument
  \return true or false
 */
inline
bool
operator==( const rcsc::AngleDeg & lhs,
            const rcsc::AngleDeg & rhs )
{
    return std::fabs( lhs.degree() - rhs.degree() ) < rcsc::AngleDeg::EPSILON;
}

/*-------------------------------------------------------------------*/
/*!
  \brief predicate operator ==
  \param lhs left hand side argument
  \param rhs right hand side argument
  \return true or false
 */
inline
bool
operator==( const rcsc::AngleDeg & lhs,
            const double rhs )
{
    return std::fabs( lhs.degree() - rhs ) < rcsc::AngleDeg::EPSILON;
}

/*-------------------------------------------------------------------*/
/*!
  \brief predicate operator ==
  \param lhs left hand side argument
  \param rhs right hand side argument
  \return true or false
 */
inline
bool
operator==( const double lhs,
            const rcsc::AngleDeg & rhs )
{
    return std::fabs( lhs - rhs.degree() ) < rcsc::AngleDeg::EPSILON;
}


/*-------------------------------------------------------------------*/
/*!
  \brief output to ostream
  \param os reference to the output stream
  \param a angle object
  \return reference to the output stream
 */
inline
std::ostream &
operator<<( std::ostream & os,
            const rcsc::AngleDeg & a )
{
    return a.print( os );
}

#endif
