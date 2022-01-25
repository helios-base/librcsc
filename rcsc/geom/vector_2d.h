// -*-c++-*-

/*!
  \file vector_2d.h
  \brief 2d vector class Header File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA, Hiroki Shimora

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

#ifndef RCSC_GEOM_VECTOR2D_H
#define RCSC_GEOM_VECTOR2D_H

#include <rcsc/geom/angle_deg.h>

#include <iostream>
#include <cmath>

namespace rcsc {

/*!
  \class Vector2D
  \brief 2D point vector class
*/
class Vector2D {
    // : public boost::addable< Vector2D >
    // , public boost::subtractable< Vector2D >
    // , public multipliable2< Vector2D, double >
    // , public dividable2< Vector2D, double >

public:

    //! constant threshold value for calculation error
    static const double EPSILON;

    //! constant error value for XY (= std::numeric_limits< doulble >::max()).
    static const double ERROR_VALUE;

    //! invalidated value vector
    static const Vector2D INVALIDATED;

    double x; //!< X coordinate
    double y; //!< Y coordinate

    /*!
      \brief default constructor.
    */
    Vector2D()
        : x( 0.0 ),
          y( 0.0 )
      { }

    /*!
      \brief create Vector with XY value directly.
      \param xx assigned x value
      \param yy assigned x value
    */
    Vector2D( const double xx,
              const double yy )
        : x( xx ),
          y( yy )
      { }

    /*!
      \brief check if this vector is valid or not.
      \return true if component values are validate.
    */
    bool isValid() const
      {
          return ( ( x != ERROR_VALUE ) && ( y != ERROR_VALUE ) );
      }

    /*!
      \brief assign XY value directly.
      \param xx assigned x value
      \param yy assigned y value
      \return reference to itself
    */
    Vector2D & assign( const double xx,
                       const double yy )
      {
          x = xx;
          y = yy;
          return *this;
      }

    /*!
      \brief assign XY value from POLAR value.
      \param radius vector's radius
      \param dir vector's angle
      \return reference to itself
    */
    Vector2D & setPolar( const double radius,
                         const AngleDeg & dir )
      {
          x = radius * dir.cos();
          y = radius * dir.sin();
          return *this;
      }

    /*!
      \brief invalidate this object
      \return this
    */
    const Vector2D & invalidate()
      {
          x = ERROR_VALUE;
          y = ERROR_VALUE;
          return *this;
      }

    /*!
      \brief get the squared length of vector.
      \return squared length
    */
    double r2() const
      {
          return x * x + y * y;
      }

    /*!
      \brief get the length of vector.
      \return length value
    */
    double r() const
      {
          //return std::hypot( x, y );
          return std::sqrt( r2() );
      }

    /*!
      \brief get the norm value. this method is equivalent to r().
      \return norm value
    */
    double norm() const
      {
          return r();
      }

    /*!
      \brief get the squared norm value. this method is equivalent to r2().
      \return squared norm value
    */
    double norm2() const
      {
          return r2();
      }

    /*!
      \brief get the length of vector. this method is equivalent to r().
      \return length value
    */
    double length() const
      {
          return r();
      }

    /*!
      \brief get the squared length value. this method is equivalent to r2().
      \return squared length value
    */
    double length2() const
      {
          return r2();
      }

    /*!
      \brief get the angle of vector.
      \return angle
    */
    AngleDeg th() const
      {
          return AngleDeg( AngleDeg::atan2_deg( y, x ) );
      }

    /*!
      \brief get the angle of vector. this method is equivalent to th().
      \return angle
    */
    AngleDeg dir() const
      {
          return th();
      }

    /*!
      \brief get new vector that XY values were set to absolute value.
      \return new vector that all values are absolute.
    */
    Vector2D abs() const
      {
          return Vector2D( std::fabs( x ), std::fabs( y ) );
      }

    /*!
      \brief get absolute x value
      \return absolute x value
    */
    double absX() const
      {
          return std::fabs( x );
      }

    /*!
      \brief get absolute y value
      \return absolute y value
    */
    double absY() const
      {
          return std::fabs( y );
      }

    /*!
      \brief add vector.
      \param v added vector
      \return reference to itself
    */
    Vector2D & add( const Vector2D & v )
      {
          x += v.x;
          y += v.y;
          return *this;
      }

    /*!
      \brief add XY values respectively.
      \param xx added x value
      \param yy added y value
      \return reference to itself
    */
    Vector2D & add( const double xx,
                    const double yy )
      {
          x += xx;
          y += yy;
          return *this;
      }

    /*!
      \brief scale this vector
      \param scalar scaling factor
      \return *this.
    */
    Vector2D & scale( const double scalar )
      {
          x *= scalar;
          y *= scalar;
          return *this;
      }

    /*!
      \brief return this vector
      \return const reference of this vector
    */
    const Vector2D & operator+() const
      {
          return *this;
      }

    /*!
      \brief create reversed vector
      \return new vector that XY values are reversed.
    */
    Vector2D operator-() const
      {
          return Vector2D( -x, -y );
      }

    /*!
      \brief add vector to itself
      \param v added vector
      \return const reference to itself
    */
    const Vector2D & operator+=( const Vector2D & v )
      {
          x += v.x;
          y += v.y;
          return *this;
      }

    /*!
      \brief subtract vector to itself
      \param v subtract argument
      \return const reference to itself
    */
    const Vector2D & operator-=( const Vector2D & v )
      {
          x -= v.x;
          y -= v.y;
          return *this;
      }

    /*!
      \brief multiplied by 'scalar'
      \param scalar multiplication argument
      \return const reference to itself
    */
    const Vector2D & operator*=( const double scalar )
      {
          //x *= scalar;
          //y *= scalar;
          //return *this;
          return scale( scalar );
      }

    /*!
      \brief divided by 'scalar'.
      \param scalar division argument
      \return const reference to itself
    */
    const Vector2D & operator/=( const double scalar )
      {
          //if ( scalar != 0 )
          if ( std::fabs( scalar ) > EPSILON )
          {
              x /= scalar;
              y /= scalar;
          }
          return *this;
      }

    /*!
      \brief get the squared distance from this to 'p'.
      \param p target point
      \return squared distance to 'p'
    */
    double dist2( const Vector2D & p ) const
      {
          //return ( Vector2D( *this ) -= p ).r2();
          return ( std::pow( this->x - p.x, 2 )
                   + std::pow( this->y - p.y, 2 ) );
      }

    /*!
      \brief get the distance from this to 'p'.
      \param p target point
      \return distance to 'p'
    */
    double dist( const Vector2D & p ) const
      {
          //return std::hypot( this->x - p.x,
          //                   this->y - p.y );
          return std::sqrt( dist2( p ) );
      }

    /*!
      \brief get the squared distance from this to (xx,yy).
      \param xx x coordinate of the target point
      \param yy y coordinate of the target point
      \return squared distance to (xx,yy)
    */
    double dist2( const double xx,
                  const double yy ) const
      {
          return ( std::pow( this->x - xx, 2 )
                   + std::pow( this->y - yy, 2 ) );
      }

    /*!
      \brief get the distance from this to (xx, yy).
      \param xx x coordinate of the target point
      \param yy y coordinate of the target point
      \return distance to (xx, yy)
    */
    double dist( const double xx,
                 const double yy ) const
      {
          //return std::hypot( this->x - xx,
          //                   this->y - yy );
          return std::sqrt( dist2( xx, yy ) );
      }

    /*!
      \brief reverse vector components
      \return *this.
    */
    Vector2D & reverse()
      {
          x = -x;
          y = -y;
          return *this;
      }

    /*!
      \brief get reversed vector.
      \return new vector object
    */
    Vector2D reversedVector() const
      {
          return Vector2D( *this ).reverse();
      }

    /*!
      \brief set vector length to 'len'.
      \param len new length to be set
      \return reference to itself
    */
    Vector2D & setLength( const double len )
      {
          double mag = this->r();
          //if ( mag == 0 )
          if ( mag < EPSILON )
          {
              return *this;
          }
          //return ( (*this) *= ( len / mag ) );
          return scale( len / mag );
      }

    /*!
      \brief get new vector that the length is set to 'len'
      \param len new length to be set
      \return new vector that the length is set to 'len'
    */
    Vector2D setLengthVector( const double len ) const
      {
          return Vector2D( *this ).setLength( len );
      }

    /*!
      \brief normalize vector. length is set to 1.0.
      \return const reference to itself
    */
    Vector2D & normalize()
      {
          return setLength( 1.0 );
      }

    /*!
      \brief get new normalized vector that the length is set to 1.0
      but angle is same
      \return new normalized vector
    */
    Vector2D normalizedVector() const
      {
          return Vector2D( *this ).normalize();
      }

    /*!
      \brief rotate this vector with 'deg'
      \param deg rotated angle by double type
      \return reference to itself
    */
    Vector2D & rotate( const double deg )
      {
          double c = std::cos( deg * AngleDeg::DEG2RAD );
          double s = std::sin( deg * AngleDeg::DEG2RAD );
          return assign( this->x * c - this->y * s,
                         this->x * s + this->y * c );
      }

    /*!
      \brief rotate this vector with 'angle'.
      \param angle rotated angle
      \return reference to itself
    */
    Vector2D & rotate( const AngleDeg & angle )
      {
          return rotate( angle.degree() );
      }

    /*!
      \brief get new vector that is rotated by 'deg'.
      \param deg rotated angle. double type.
      \return new vector rotated by 'deg'
    */
    Vector2D rotatedVector( const double deg ) const
      {
          return Vector2D( *this ).rotate( deg );
      }

    /*!
      \brief get new vector that is rotated by 'angle'.
      \param angle rotated angle.
      \return new vector rotated by 'angle'
    */
    Vector2D rotatedVector( const AngleDeg & angle ) const
      {
          return Vector2D( *this ).rotate( angle.degree() );
      }

    /*!
      \brief set vector's angle to 'angle'
      \param dir new angle to be set
      \return reference to itself
    */
    Vector2D & setDir( const AngleDeg & dir )
      {
          double radius = this->r();
          x = radius * dir.cos();
          y = radius * dir.sin();
          return *this;
      }

    /*!
      \brief get inner(dot) product with 'v'.
      \param v target vector
      \return value of inner product
    */
    double innerProduct( const Vector2D & v ) const
      {
          return this->x * v.x + this->y * v.y;
          // ==  |this| * |v| * (*this - v).th().cos()
      }

    /*!
      \brief get virtal outer(cross) product with 'v'.
      \param v target vector
      \return value of outer product
    */
    double outerProduct( const Vector2D & v ) const
      {
          /*---------------------*
           * assume virtual 3D environment.
           * calculate Z-coordinate of outer product in right hand orientation.
           * For the time being, Input Vector's Z-coordinate is set to ZERO.
           *---------------------*/
          // Normal 3D outer product
          //   xn = this->y * v.z - this->z * v.y;
          //   yn = this->z * v.x - this->x * v.z;
          // # zn = this->x * v.y - this->y * v.x;
          return this->x * v.y - this->y * v.x;
          // == |this| * |v| * (*this - v).th().sin()
      }

    /*!
      \brief check if this vector is strictly same as given vector.
      \param other compared vector
      \return true if strictly same, otherwise false.
    */
    bool equals( const Vector2D & other ) const
      {
          return this->x == other.x
              && this->y == other.y;
      }

    /*!
      \brief check if this vector is weakly same as given vector.
      \param other compared vector.
      \return true if weakly same, otherwise false.
    */
    bool equalsWeakly( const Vector2D & other ) const
      {
          //return dist2( other ) < EPSILON * EPSILON;
          return std::fabs( this->x - other.x ) < EPSILON
                                                  && std::fabs( this->y - other.y ) < EPSILON;
      }

    //////////////////////////////////////////////
    // static utility

    /*!
      \brief get new Vector created by POLAR value.
      \param mag length of vector
      \param theta angle of vector
      \return new vector object
    */
    inline
    static
    Vector2D polar2vector( const double mag,
                           const AngleDeg & theta )
      {
          return Vector2D( mag * theta.cos(), mag * theta.sin() );
      }

    /*!
      \brief get new Vector created by POLAR value.
      \param mag length of vector
      \param theta angle of vector
      \return new vector object
    */
    inline
    static
    Vector2D from_polar( const double mag,
                         const AngleDeg & theta )
      {
          return Vector2D( mag * theta.cos(), mag * theta.sin() );
      }

    /*!
      \brief get inner(dot) product for v1 and v2.
      \param v1 input 1
      \param v2 input 2
      \return value of inner product
    */
    inline
    static
    double inner_product( const Vector2D & v1,
                          const Vector2D & v2 )
      {
          return v1.innerProduct( v2 );
      }

    /*!
      \brief get outer(cross) product for v1 and v2.
      \param v1 input 1
      \param v2 input 2
      \return value of outer product
    */
    inline
    static
    double outer_product( const Vector2D & v1,
                          const Vector2D & v2 )
      {
          return v1.outerProduct( v2 );
      }

    //////////////////////////////////////////////
    // stream utility

    /*!
      \brief output XY values to ostream.
      \param os reference to ostream
      \return reference to ostream
    */
    std::ostream & print( std::ostream & os ) const
      {
          os << '(' << x << ", " << y << ')';
          return os;
      }

    /*!
      \brief output rounded XY values to ostream.
      \param os reference to ostream
      \param prec precision of output value
      \return reference to ostream
    */
    std::ostream & printRound( std::ostream & os,
                               const double prec = 0.1 ) const
      {
          os << '('  << rint( x / prec ) * prec
             << ", " << rint( y / prec ) * prec << ')';
          return os;
      }

    //////////////////////////////////////////////
    // functors for comparison

    /*!
      \class XCmp
      \brief comparison predicate for X value.
    */
    struct XCmp {
        /*!
          \brief functional operator.
          \param lhs left hand side argument.
          \param rhs right hand side argument.
          \return compared result.
        */
        bool operator()( const Vector2D & lhs,
                         const Vector2D & rhs ) const
          {
              return lhs.x < rhs.x;
          }
    };

    /*!
      \struct YCmp
      \brief comparison predicate for Y value.
    */
    struct YCmp {
        /*!
          \brief functional operator.
          \param lhs left hand side argument.
          \param rhs right hand side argument.
          \return compared result.
        */
        bool operator()( const Vector2D & lhs,
                         const Vector2D & rhs ) const
          {
              return lhs.y < rhs.y;
          }
    };

    /*!
      \struct AbsXCmp
      \brief comparison predicate for absolute X value.
    */
    struct AbsXCmp {
        /*!
          \brief functional operator.
          \param lhs left hand side argument.
          \param rhs right hand side argument.
          \return compared result.
        */
        bool operator()( const Vector2D & lhs,
                         const Vector2D & rhs ) const
          {
              return lhs.absX() < rhs.absX();
          }
    };

    /*!
      \struct AbsYCmp
      \brief comparison predicate for absolute Y value.
    */
    struct AbsYCmp {
        /*!
          \brief functional operator.
          \param lhs left hand side argument.
          \param rhs right hand side argument.
          \return compared result.
        */
        bool operator()( const Vector2D & lhs,
                         const Vector2D & rhs ) const
          {
              return lhs.absY() < rhs.absY();
          }
    };

    /*!
      \struct XYCmp
      \brief comparison predicate for XY value (X -> Y order).
    */
    struct XYCmp {
        /*!
          \brief functional operator.
          \param lhs left hand side argument.
          \param rhs right hand side argument.
          \return compared result.
        */
        bool operator()( const Vector2D & lhs,
                         const Vector2D & rhs ) const
          {
              return ( lhs.x < rhs.x
                       ? true
                       : lhs.x > rhs.x
                       ? false
                       : lhs.y < rhs.y );
          }

    };

    /*!
      \struct YXCmp
      \brief comparison predicatio for XY value (Y -> X order)
    */
    struct YXCmp {
        /*!
          \brief functional operator.
          \param lhs left hand side argument.
          \param rhs right hand side argument.
          \return compared result.
        */
        bool operator()( const Vector2D & lhs,
                         const Vector2D & rhs ) const
          {
              return ( lhs.y < rhs.y
                       || ( lhs.y == rhs.y && lhs.x < rhs.x ) );
          }
    };

};

} // end of namespace


////////////////////////////////////////////////////////
// comparison operators
/*!
  \brief check vectors are strictly same or not.
  \param lhs left hand side parameter
  \param rhs right hand side parameter
  \return true if vectors are same.
*/
inline
bool
operator==( const rcsc::Vector2D & lhs,
            const rcsc::Vector2D & rhs )
{
    //return lhs.x == rhs.x
    //    && lhs.y == rhs.y;
    return lhs.equals( rhs );
}

/*!
  \brief check vectors are strictly different or not.
  \param lhs left hand side parameter
  \param rhs right hand side parameter
  \return true if vectors are not same.
*/
inline
bool
operator!=( const rcsc::Vector2D & lhs,
            const rcsc::Vector2D & rhs )
{
    return ! operator==( lhs, rhs );
}


////////////////////////////////////////////////////////
// arithmetic operators

/*!
  \brief operator add(T, T)
  \param lhs left hand side parameter
  \param rhs right hand side parameter
  \return new vector object
*/
inline
const
rcsc::Vector2D
operator+( const rcsc::Vector2D & lhs,
           const rcsc::Vector2D & rhs )
{
    return rcsc::Vector2D( lhs ) += rhs;
}

/*!
  \brief operator sub(T, T)
  \param lhs left hand side parameter
  \param rhs right hand side parameter
  \return new vector object
*/
inline
const
rcsc::Vector2D
operator-( const rcsc::Vector2D & lhs,
           const rcsc::Vector2D & rhs )
{
    return rcsc::Vector2D( lhs ) -= rhs;
}

/*!
  \brief operator mult(T, U)
  \param lhs left hand side parameter
  \param rhs right hand side parameter. double type
  \return new vector object
*/
inline
const
rcsc::Vector2D
operator*( const rcsc::Vector2D & lhs,
           const double rhs )
{
    return rcsc::Vector2D( lhs ) *= rhs;
}

/*!
  \brief operator div(T, U)
  \param lhs left hand side parameter
  \param rhs right hand side parameter. double type
  \return new vector object
*/
inline
const
rcsc::Vector2D
operator/( const rcsc::Vector2D & lhs,
           const double rhs )
{
    return rcsc::Vector2D( lhs ) /= rhs;
}

/*!
  \brief never used
*/
template < typename T >
bool
operator<( const rcsc::Vector2D & lhs,
           const T & rhs );

/*!
  \brief never used
*/
template < typename T >
bool
operator<=( const rcsc::Vector2D & lhs,
            const T & rhs );

/*!
  \brief never used
*/
template < typename T >
bool
operator>( const rcsc::Vector2D & lhs,
           const T & rhs );

/*!
  \brief never used
*/
template < typename T >
bool
operator>=( const rcsc::Vector2D & lhs,
            const T & rhs );

/*!
  \brief never used
*/
template < typename T >
bool
operator<( const T & lhs,
           const rcsc::Vector2D & rhs );


/*!
  \brief never used
*/
template < typename T >
bool
operator<=(const T & lhs,
           const rcsc::Vector2D & rhs );


/*!
  \brief never used
*/
template < typename T >
bool
operator>( const T & lhs,
           const rcsc::Vector2D & rhs );

/*!
  \brief never used
*/
template < typename T >
bool
operator>=( const T & lhs,
            const rcsc::Vector2D & rhs );

/*!
  \brief never used
*/
template < typename T >
bool
operator==( const T & lhs,
            const rcsc::Vector2D & rhs );

/*!
  \brief never used
*/
template < typename T >
bool
operator!=( const T & lhs,
            const rcsc::Vector2D & rhs );



////////////////////////////////////////////////////////

/*!
  \brief stream operator
  \param os reference to ostream
  \param v output value
  \return reference to ostream
*/
inline
std::ostream &
operator<<( std::ostream & os,
            const rcsc::Vector2D & v )
{
    return v.print( os );
}


#endif
