// -*-c++-*-

/*!
  \file soccer_math.cpp
  \brief math utility depending on RCSSServer2D Sourcd File
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

#include <rcsc/soccer_math.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
wind_effect( const double & speed,
             const double & weight,
             const double & wind_force,
             const double & wind_dir,
             const double & wind_weight,
             const double & wind_rand,
             Vector2D * wind_error )
{
    /*
    // rcssserver/src/object.C
    PVector MPObject::wind()
    {
    return PVector( vel.r()
    * ( weather->wind_vector.x +
    drand(-weather->wind_rand, weather->wind_rand) )
    / (weight * WIND_WEIGHT),
    vel.r()
    * ( weather->wind_vector.y +
    drand(-weather->wind_rand, weather->wind_rand) )
    / (weight * WIND_WEIGHT) );
    }
    // rcssserver/srd/param.h
    #define WIND_WEIGHT                10000.0
    */

    Vector2D wind_vec = Vector2D::polar2vector( wind_force, wind_dir );

    if ( wind_error )
    {
        wind_error->assign( speed * wind_vec.x * wind_rand
                            / ( weight * wind_weight ),
                            speed * wind_vec.y * wind_rand
                            / ( weight * wind_weight ) );
    }

    return Vector2D( speed * wind_vec.x / ( weight * wind_weight ),
                     speed * wind_vec.y / ( weight * wind_weight ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
double
unquantize_error( const double & see_dist,
                  const double & qstep )
{
    double min_dist
        = ( std::exp( unquantize_min
                      ( std::log( unquantize_min( see_dist, 0.1 ) ), qstep ) )
            - SERVER_EPS );
    double max_dist
        = ( std::exp( unquantize_max
                      ( std::log( unquantize_max( see_dist, 0.1 ) ), qstep ) )
            - SERVER_EPS );

    return std::fabs( max_dist - min_dist );
}

} // end namespace

/*
  ABOUT DIR ERROR

  from rcssserver/src/utility.h
  #define Rad2IDeg(a) ((int)(Rad2Deg(a)))
  #define Rad2IDegRound(a) ((int)rint(Rad2Deg(a)))

  int VisualSensor_v1
  ::obj_dir_data_t
  ::calcDegDir ( const double& rad )
  {
  return Rad2IDeg ( rad );
  }

  int VisualSensor_v7
  ::obj_dir_data_t
  ::calcDegDir ( const double& rad )
  {
  return Rad2IDegRound ( rad );
  }
*/


/*
  ABOUT DIST ERROR

  see UvA Trilearn source code.
  WorldModelUpdate.C

  bool WorldModel::getMinMaxDistQuantizeValue( double dOutput, double *dMin,
  double *dMax,   double x1, double x2 )
  {
  // change output a little bit to circumvent boundaries
  // q = quantize(e^(quantize(ln(V),x1)),x2) with quantize(V,Q) = rint(V/Q)*Q
  // e^(quantize(ln(V),x1)_min = invQuantize( q, x2 )
  // quantize(ln(V),x1) = ln ( invQuantize( q, x2 ) )
  // ln(V) = invQuantize( ln ( invQuantize( q, x2 ) ), x1 )
  // V_min = e^( invQuantize( ln ( invQuantize( q, x2 ) ), x1 ) )
  // apply inverse quantize twice to get correct value

  dOutput -= SERVER_EPS;
  *dMin = exp(invQuantizeMin( log( invQuantizeMin(dOutput,x2) ), x1 ));
  dOutput += SERVER_EPS * 2.0;
  *dMax = exp(invQuantizeMax( log( invQuantizeMax(dOutput,x2) ), x1 ));
  return true;
  }

  double WorldModel::invQuantizeMin( double dOutput, double dQuantizeStep )
  {
  // q = quantize( V, Q ) = rint(V/Q)*Q -> q/Q = rint( V/Q)
  // min = rint(q/Q)-0.5 = V_min/Q -> V_min = (rint(q/Q)-0.5)*Q
  return (rint( dOutput / dQuantizeStep )-0.5 )*dQuantizeStep;
  }

  double WorldModel::invQuantizeMax( double dOutput, double dQuantizeStep )
  {
  // q = quantize( V, Q ) = rint(V/Q)*Q -> q/Q = rint( V/Q)
  // max = rint(q/Q)+0.5 = V_max/Q -> V_max = (rint(q/Q)-0.5)*Q
  return (rint( dOutput/dQuantizeStep) + 0.5 )*dQuantizeStep;
  }


*/
