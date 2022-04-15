// -*-c++-*-

/*!
  \file soccer_math.h
  \brief math utility depending on RCSSServer2D Header File
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

#ifndef RCSC_SOCCERMATH_H
#define RCSC_SOCCERMATH_H

#include <rcsc/geom/vector_2d.h>

namespace rcsc {

//! epsilon value
constexpr double SERVER_EPS = 1.0e-10;

///////////////////////////////////////////////////////////////////////
// kick command related

/*-------------------------------------------------------------------*/
/*!
  \brief calculate kick rate
  \param dist distance from player to ball
  \param dir_diff angle difference from player's body to ball
  \param kprate player's kick power rate parameter
  \param bsize ball radius
  \param psize player radius
  \param kmargin player's kickable area margin
  \return rate of the kick power effect

  it may be useful to redefine this algorighm in kick module
*/
inline
double
kick_rate( const double & dist,
           const double & dir_diff,
           const double & kprate,
           const double & bsize,
           const double & psize,
           const double & kmargin )
{
    return kprate * ( 1.0
                      - 0.25 * std::fabs( dir_diff ) / 180.0
                      - 0.25 * ( dist - bsize - psize ) / kmargin );
}

///////////////////////////////////////////////////////////////////////
// dash command related

/*-------------------------------------------------------------------*/
/*!
  \brief calculate effective dash power rate according to the input dash direction
  \param dir relative dash direction
  \param back_dash_rate server parameter
  \param side_dash_rate server parameter
  \return effective dash power rate
 */
inline
double
dir_rate( const double & dir,
          const double & back_dash_rate,
          const double & side_dash_rate )
{
    return ( std::fabs( dir ) > 90.0
             ? back_dash_rate - ( ( back_dash_rate - side_dash_rate )
                                  * ( 1.0 - ( std::fabs( dir ) - 90.0 ) / 90.0 )
                                  )
             : side_dash_rate + ( ( 1.0 - side_dash_rate )
                                  * ( 1.0 - std::fabs( dir ) / 90.0 ) )
             );
}

///////////////////////////////////////////////////////////////////////
// turn command related

/*-------------------------------------------------------------------*/
/*!
  \brief calculate effective turn moment.
  it may be useful to redefine this algorighm in movement action module
  \param turn_moment value used by turn command
  \param speed player's current speed
  \param inertiamoment player's inertia moment parameter
  \return calculated actual turn angle
*/
inline
double
effective_turn( const double & turn_moment,
                const double & speed,
                const double & inertiamoment )
{
    return turn_moment / ( 1.0 + inertiamoment * speed );
}

///////////////////////////////////////////////////////////////////////
// dash command related

/*-------------------------------------------------------------------*/
/*!
  \brief calculate converged max speed, when using "dash_power"
  \param dash_power value used by dash command
  \param dprate player's dash power rate parameter
  \param effort player's effort parameter
  \param decay player's decay parameter
  \return achieved final speed

  NOTE: returned value should be compared with player_speed_max parameter
*/
inline
double
final_speed( const double & dash_power,
             const double & dprate,
             const double & effort,
             const double & decay )
{
    // if player continue to run using the same dash power
    // archieved speed at n step later is sum of infinite geometric series

    // !! NOTE !!
    // You must compare this value to the PlayerType::playerSpeedMax()

    //return ( (dash_power * dprate * effort) // == accel
    //         * (1.0 / (1.0 - decay)) ); // == sum inf geom series
    return ( ( std::fabs( dash_power ) * dprate * effort ) // == accel
             / ( 1.0 - decay ) ); // == sum inf geom series
}

/*-------------------------------------------------------------------*/
/*!
  \brief check if player's poteantial max speed is over plsyer_speed_max
  parameter.
  \param dash_power value used by dash command
  \param dprate plaeyr's dash power rate parameter
  \param effort player's effort parameter
  \param speed_max player's limited speed parameter
  \param decay player's decay parameter
  \return true, if player can over player_speed_max
*/
inline
bool
can_over_speed_max( const double & dash_power,
                    const double & dprate,
                    const double & effort,
                    const double & decay,
                    const double & speed_max )

{
    return ( std::fabs( dash_power ) * dprate * effort // max accel
             > speed_max * ( 1.0 - decay ) ); // is over speed decay
}

///////////////////////////////////////////////////////////////////////
// predictor method for inertia movement

/*-------------------------------------------------------------------*/
/*!
  \brief estimate future travel after n steps only by inertia.
  No additional acceleration.
  \param initial_vel object's first velocity
  \param n_step number of total steps
  \param decay object's decay parameter
  \return vector of total travel
*/
inline
Vector2D
inertia_n_step_travel( const Vector2D & initial_vel,
                       const int n_step,
                       const double & decay )
{
    return Vector2D( initial_vel )
        *= ( ( 1.0 - std::pow( decay, n_step ) ) / ( 1.0 - decay ) );
}

/*-------------------------------------------------------------------*/
/*!
  \brief estimate future point after n steps only by inertia.
  No additional acceleration
  \param initial_pos object's first position
  \param initial_vel object's first velocity
  \param n_step number of total steps
  \param decay object's decay parameter
  \return coordinate of the reached point
*/
inline
Vector2D
inertia_n_step_point( const Vector2D & initial_pos,
                      const Vector2D & initial_vel,
                      const int n_step,
                      const double & decay )
{
    /*
      return
      initial_pos
      + initial_vel * ( (1.0 - std::pow(decay, static_cast<double>(n_step)))
      / (1.0 - decay) );
    */
    /*
      return Vector2D(initial_vel
      * (1.0 - std::pow(decay, n_step) / (1.0 - decay)))
      += initial_pos;
    */
    /*
      double rate = ( (1.0 - std::pow(decay, static_cast<double>(n_step)))
      / (1.0 - decay) );
      return initial_pos + ( initial_vel * rate );
    */
    return Vector2D( initial_pos )
        += inertia_n_step_travel( initial_vel, n_step, decay );
}

/*-------------------------------------------------------------------*/
/*!
  \brief estimate travel distance only by inertia.
  No additional acceleration
  \param initial_speed object's first speed
  \param n_step number of total steps
  \param decay object's decay parameter
  \return total travel distance
*/
inline
double
inertia_n_step_distance( const double & initial_speed,
                         const int n_step,
                         const double & decay )
{
    return initial_speed
        * ( 1.0 - std::pow( decay, n_step ) )
        / ( 1.0 - decay );
}

/*-------------------------------------------------------------------*/
/*!
  \brief estimate travel distance only by inertia.
  No additional accel
  \param initial_speed object's first speed
  \param n_step_real number of total steps(floating point number)
  \param decay object's decay parameter
  \return total travel distance
*/
inline
double
inertia_n_step_distance( const double & initial_speed,
                         const double & n_step_real,
                         const double & decay )
{
    return initial_speed
        * ( 1.0 - std::pow( decay, n_step_real ) )
        / ( 1.0 - decay );
}


/*-------------------------------------------------------------------*/
/*!
  \brief calculate total travel only by inertia movement.
  \param initial_vel object's first velocity
  \param decay object's decay parameter
  \return final travel vector
*/
inline
Vector2D
inertia_final_travel( const Vector2D & initial_vel,
                      const double & decay )
{
    return Vector2D( initial_vel ) /= ( 1.0 - decay );
}

/*-------------------------------------------------------------------*/
/*!
  \brief calculate final reach point only by inertia.
  \param initial_pos object's first position
  \param initial_vel object's first velocity
  \param decay object's decay parameter
  \return coordinate of the reached point
*/
inline
Vector2D
inertia_final_point( const Vector2D & initial_pos,
                     const Vector2D & initial_vel,
                     const double & decay )
{
    /*
      return Vector2D(initial_vel / (1.0 - decay)) += initial_pos;
    */
    /*
      Vector2D end_pos(initial_vel);
      end_pos /= (1.0 - decay);
      return end_pos += initial_pos;
    */
    /*
      return initial_pos + ( initial_vel / (1.0 - decay) );
    */
    return Vector2D( initial_pos )
        += inertia_final_travel( initial_vel, decay );
}

/*-------------------------------------------------------------------*/
/*!
  \brief calculate total travel distance only by inertia.
  \param initial_speed object's first speed
  \param decay object's decay parameter
  \return distance value that the object reaches
*/
inline
double
inertia_final_distance( const double & initial_speed,
                        const double & decay )
{
    return initial_speed / ( 1.0 - decay );
}

///////////////////////////////////////////////////////////////////////
// localization

/*-------------------------------------------------------------------*/
/*!
  \brief quantize a floating point number
  \param value value to be rounded
  \param qstep round precision
  \return rounded value

  same as define Quantize(v,q) ((rint((v)/(q)))*(q))
*/
inline
double
quantize( const double & value,
          const double & qstep )
{
    return rint( value / qstep ) * qstep;
}

/*-------------------------------------------------------------------*/
/*!
  \brief calculate quantized distance value about dist quantization
  \param unq_dist actual distance
  \param qstep server parameter
  \return quantized distance

  [rcssserver/src/utility.h]
  define EPS 1.0e-10
  define Quantize(v,q)  ((rint((v)/(q)))*(q))

  [rcssserver/src/visualsensor.cc]
  double VisualSensor_v1
  ::obj_dist_data_t
  ::calcQuantDist(const double& dist, const double& qstep )
  {
  return Quantize(exp(Quantize(log(dist + EPS), qstep)), 0.1);
  }
*/
inline
double
quantize_dist( const double & unq_dist,
               const double & qstep )
{
    return quantize( std::exp
                     ( quantize( std::log
                                 ( unq_dist + SERVER_EPS ), qstep ) ), 0.1 );
}

/*-------------------------------------------------------------------*/
/*!
  \brief calculate minnimal value by inverse quantize function
  \param dist quantized distance
  \param qstep server parameter
  \return minimal distance wiothin unquantized distance range
*/
inline
double
unquantize_min( const double & dist,
                const double & qstep )
{
    return ( rint( dist / qstep ) - 0.5 ) * qstep;
}

/*-------------------------------------------------------------------*/
/*!
  \brief calculate maximal value by inverse quantize function
  \param dist quantized distance
  \param qstep server parameter
  \return maximal distance wiothin unquantized distance range
*/
inline
double
unquantize_max( const double & dist,
                const double & qstep )
{
    return ( rint( dist / qstep ) + 0.5 ) * qstep;
}

/*-------------------------------------------------------------------*/
/*!
  \brief calculate wind effect
  \param speed current object's speed
  \param weight object's speed
  \param wind_force server parameter
  \param wind_dir server parameter
  \param wind_weight server parameter
  \param wind_rand server parameter
  \param wind_error error value that is calculated by this method
  \return wind effect accelaration
*/
Vector2D
wind_effect( const double & speed,
             const double & weight,
             const double & wind_force,
             const double & wind_dir,
             const double & wind_weight,
             const double & wind_rand,
             Vector2D * wind_error );

/*-------------------------------------------------------------------*/
/*!
  \brief calculate minmax error range by inverse quantize function
  \param see_dist seen(quantized) distance
  \param qstep server parameter
  \return error value of inverse unquantized distance
*/
double
unquantize_error( const double & see_dist,
                  const double & qstep );

} // end namespace

#endif
