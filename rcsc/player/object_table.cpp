// -*-c++-*-

/*!
  \file object_table.cpp
  \brief seen object distance error table Source File
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

#include "object_table.h"

#include <rcsc/common/server_param.h>

#include <algorithm>
#include <iostream>
#include <cmath>

namespace rcsc {

namespace {

struct DataEntryCmp {
    bool operator()( const ObjectTable::DataEntry & lhs,
                     const ObjectTable::DataEntry & rhs ) const
      {
          return lhs.M_seen_dist < rhs.M_seen_dist;
      }
};

}

const double ObjectTable::SERVER_EPS = 1.0e-10;

/*-------------------------------------------------------------------*/
/*!

 */
ObjectTable::ObjectTable()
{
    M_static_table.reserve( 400 );
    M_movable_table.reserve( 70 );

    createLandmarkMap();

    createTable();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ObjectTable::createLandmarkMap()
{
    ///////////////////////////////////////////////////////////////////////
    const double pitch_half_w   = ServerParam::i().pitchHalfWidth();
    const double pitch_half_l   = ServerParam::i().pitchHalfLength();
    const double penalty_l      = ServerParam::i().penaltyAreaLength();
    const double penalty_half_w = ServerParam::i().penaltyAreaHalfWidth();
    const double goal_half_w    = ServerParam::i().goalHalfWidth();
    ///////////////////////////////////////////////////////////////////////

    M_landmark_map[Goal_L]    = Vector2D( -pitch_half_l,   0.0 );
    M_landmark_map[Goal_R]    = Vector2D( +pitch_half_l,   0.0 );

    M_landmark_map[Flag_C]    = Vector2D(           0.0,           0.0 );
    M_landmark_map[Flag_CT]   = Vector2D(           0.0, -pitch_half_w );
    M_landmark_map[Flag_CB]   = Vector2D(           0.0, +pitch_half_w );
    M_landmark_map[Flag_LT]   = Vector2D( -pitch_half_l, -pitch_half_w );
    M_landmark_map[Flag_LB]   = Vector2D( -pitch_half_l, +pitch_half_w );
    M_landmark_map[Flag_RT]   = Vector2D( +pitch_half_l, -pitch_half_w );
    M_landmark_map[Flag_RB]   = Vector2D( +pitch_half_l, +pitch_half_w );

    M_landmark_map[Flag_PLT]  = Vector2D( -(pitch_half_l - penalty_l), -penalty_half_w );
    M_landmark_map[Flag_PLC]  = Vector2D( -(pitch_half_l - penalty_l),             0.0 );
    M_landmark_map[Flag_PLB]  = Vector2D( -(pitch_half_l - penalty_l), +penalty_half_w );
    M_landmark_map[Flag_PRT]  = Vector2D( +(pitch_half_l - penalty_l), -penalty_half_w );
    M_landmark_map[Flag_PRC]  = Vector2D( +(pitch_half_l - penalty_l),             0.0 );
    M_landmark_map[Flag_PRB]  = Vector2D( +(pitch_half_l - penalty_l), +penalty_half_w );

    M_landmark_map[Flag_GLT]  = Vector2D( -pitch_half_l, -goal_half_w );
    M_landmark_map[Flag_GLB]  = Vector2D( -pitch_half_l, +goal_half_w );
    M_landmark_map[Flag_GRT]  = Vector2D( +pitch_half_l, -goal_half_w );
    M_landmark_map[Flag_GRB]  = Vector2D( +pitch_half_l, +goal_half_w );

    M_landmark_map[Flag_TL50] = Vector2D( -50.0, -pitch_half_w - 5.0 );
    M_landmark_map[Flag_TL40] = Vector2D( -40.0, -pitch_half_w - 5.0 );
    M_landmark_map[Flag_TL30] = Vector2D( -30.0, -pitch_half_w - 5.0 );
    M_landmark_map[Flag_TL20] = Vector2D( -20.0, -pitch_half_w - 5.0 );
    M_landmark_map[Flag_TL10] = Vector2D( -10.0, -pitch_half_w - 5.0 );

    M_landmark_map[Flag_T0]   = Vector2D(  0.0, -pitch_half_w - 5.0 );

    M_landmark_map[Flag_TR10] = Vector2D( +10.0, -pitch_half_w - 5.0 );
    M_landmark_map[Flag_TR20] = Vector2D( +20.0, -pitch_half_w - 5.0 );
    M_landmark_map[Flag_TR30] = Vector2D( +30.0, -pitch_half_w - 5.0 );
    M_landmark_map[Flag_TR40] = Vector2D( +40.0, -pitch_half_w - 5.0 );
    M_landmark_map[Flag_TR50] = Vector2D( +50.0, -pitch_half_w - 5.0 );

    M_landmark_map[Flag_BL50] = Vector2D( -50.0,  pitch_half_w + 5.0 );
    M_landmark_map[Flag_BL40] = Vector2D( -40.0,  pitch_half_w + 5.0 );
    M_landmark_map[Flag_BL30] = Vector2D( -30.0,  pitch_half_w + 5.0 );
    M_landmark_map[Flag_BL20] = Vector2D( -20.0,  pitch_half_w + 5.0 );
    M_landmark_map[Flag_BL10] = Vector2D( -10.0,  pitch_half_w + 5.0 );

    M_landmark_map[Flag_B0]   = Vector2D(   0.0,  pitch_half_w + 5.0);

    M_landmark_map[Flag_BR10] = Vector2D( +10.0,  pitch_half_w + 5.0 );
    M_landmark_map[Flag_BR20] = Vector2D( +20.0,  pitch_half_w + 5.0 );
    M_landmark_map[Flag_BR30] = Vector2D( +30.0,  pitch_half_w + 5.0 );
    M_landmark_map[Flag_BR40] = Vector2D( +40.0,  pitch_half_w + 5.0 );
    M_landmark_map[Flag_BR50] = Vector2D( +50.0,  pitch_half_w + 5.0 );

    M_landmark_map[Flag_LT30] = Vector2D( -pitch_half_l - 5.0, -30.0 );
    M_landmark_map[Flag_LT20] = Vector2D( -pitch_half_l - 5.0, -20.0 );
    M_landmark_map[Flag_LT10] = Vector2D( -pitch_half_l - 5.0, -10.0 );

    M_landmark_map[Flag_L0]   = Vector2D( -pitch_half_l - 5.0,   0.0 );

    M_landmark_map[Flag_LB10] = Vector2D( -pitch_half_l - 5.0,  10.0 );
    M_landmark_map[Flag_LB20] = Vector2D( -pitch_half_l - 5.0,  20.0 );
    M_landmark_map[Flag_LB30] = Vector2D( -pitch_half_l - 5.0,  30.0 );

    M_landmark_map[Flag_RT30] = Vector2D( +pitch_half_l + 5.0, -30.0 );
    M_landmark_map[Flag_RT20] = Vector2D( +pitch_half_l + 5.0, -20.0 );
    M_landmark_map[Flag_RT10] = Vector2D( +pitch_half_l + 5.0, -10.0 );

    M_landmark_map[Flag_R0]   = Vector2D( +pitch_half_l + 5.0,   0.0 );

    M_landmark_map[Flag_RB10] = Vector2D( +pitch_half_l + 5.0,  10.0 );
    M_landmark_map[Flag_RB20] = Vector2D( +pitch_half_l + 5.0,  20.0 );
    M_landmark_map[Flag_RB30] = Vector2D( +pitch_half_l + 5.0,  30.0 );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
ObjectTable::createTable()
{
    M_static_table.clear();
    M_movable_table.clear();

    M_static_table.push_back( DataEntry( 0.00, 0.025019, 0.025019 ) );
    M_static_table.push_back( DataEntry( 0.10, 0.100178, 0.050142 ) );
    M_static_table.push_back( DataEntry( 0.20, 0.200321, 0.050003 ) );
    M_static_table.push_back( DataEntry( 0.30, 0.301007, 0.050684 ) );
    M_static_table.push_back( DataEntry( 0.40, 0.401636, 0.049945 ) );
    M_static_table.push_back( DataEntry( 0.50, 0.501572, 0.049991 ) );
    M_static_table.push_back( DataEntry( 0.60, 0.599413, 0.047852 ) );
    M_static_table.push_back( DataEntry( 0.70, 0.699639, 0.052376 ) );
    M_static_table.push_back( DataEntry( 0.80, 0.799954, 0.047940 ) );
    M_static_table.push_back( DataEntry( 0.90, 0.897190, 0.049297 ) );
    M_static_table.push_back( DataEntry( 1.00, 0.996257, 0.049771 ) );
    M_static_table.push_back( DataEntry( 1.10, 1.095282, 0.049255 ) );
    M_static_table.push_back( DataEntry( 1.20, 1.198429, 0.053893 ) );
    M_static_table.push_back( DataEntry( 1.30, 1.304474, 0.052152 ) );
    M_static_table.push_back( DataEntry( 1.40, 1.405808, 0.049183 ) );
    M_static_table.push_back( DataEntry( 1.50, 1.499977, 0.044986 ) );
    M_static_table.push_back( DataEntry( 1.60, 1.600974, 0.056011 ) );
    M_static_table.push_back( DataEntry( 1.70, 1.699463, 0.042478 ) );
    M_static_table.push_back( DataEntry( 1.80, 1.795798, 0.053858 ) );
    M_static_table.push_back( DataEntry( 1.90, 1.897073, 0.047417 ) );
    M_static_table.push_back( DataEntry( 2.00, 1.994338, 0.049849 ) );
    M_static_table.push_back( DataEntry( 2.10, 2.096590, 0.052405 ) );
    M_static_table.push_back( DataEntry( 2.20, 2.204085, 0.055091 ) );
    M_static_table.push_back( DataEntry( 2.30, 2.305275, 0.046100 ) );
    M_static_table.push_back( DataEntry( 2.40, 2.399355, 0.047981 ) );
    M_static_table.push_back( DataEntry( 2.50, 2.497274, 0.049940 ) );
    M_static_table.push_back( DataEntry( 2.60, 2.599191, 0.051978 ) );
    M_static_table.push_back( DataEntry( 2.70, 2.705266, 0.054099 ) );
    M_static_table.push_back( DataEntry( 2.80, 2.801381, 0.042018 ) );
    M_static_table.push_back( DataEntry( 2.90, 2.901419, 0.058021 ) );
    M_static_table.push_back( DataEntry( 3.00, 3.004504, 0.045065 ) );
    M_static_table.push_back( DataEntry( 3.10, 3.096005, 0.046437 ) );
    M_static_table.push_back( DataEntry( 3.20, 3.190292, 0.047851 ) );
    M_static_table.push_back( DataEntry( 3.30, 3.287451, 0.049309 ) );
    M_static_table.push_back( DataEntry( 3.40, 3.387569, 0.050810 ) );
    M_static_table.push_back( DataEntry( 3.50, 3.490736, 0.052358 ) );
    M_static_table.push_back( DataEntry( 3.60, 3.597044, 0.053952 ) );
    M_static_table.push_back( DataEntry( 3.70, 3.706591, 0.055596 ) );
    M_static_table.push_back( DataEntry( 3.80, 3.800186, 0.038001 ) );
    M_static_table.push_back( DataEntry( 3.90, 3.896632, 0.058446 ) );
    M_static_table.push_back( DataEntry( 4.00, 3.995026, 0.039950 ) );
    M_static_table.push_back( DataEntry( 4.10, 4.096416, 0.061442 ) );
    M_static_table.push_back( DataEntry( 4.20, 4.199855, 0.041998 ) );
    M_static_table.push_back( DataEntry( 4.30, 4.306444, 0.064592 ) );
    M_static_table.push_back( DataEntry( 4.40, 4.415186, 0.044151 ) );
    M_static_table.push_back( DataEntry( 4.50, 4.504379, 0.045043 ) );
    M_static_table.push_back( DataEntry( 4.60, 4.595374, 0.045953 ) );
    M_static_table.push_back( DataEntry( 4.70, 4.688206, 0.046881 ) );
    M_static_table.push_back( DataEntry( 4.80, 4.782914, 0.047828 ) );
    M_static_table.push_back( DataEntry( 4.90, 4.879536, 0.048795 ) );
    M_static_table.push_back( DataEntry( 5.00, 4.978109, 0.049780 ) );
    M_static_table.push_back( DataEntry( 5.10, 5.078673, 0.050786 ) );
    M_static_table.push_back( DataEntry( 5.20, 5.181269, 0.051812 ) );
    M_static_table.push_back( DataEntry( 5.30, 5.285938, 0.052859 ) );
    M_static_table.push_back( DataEntry( 5.40, 5.392721, 0.053926 ) );
    M_static_table.push_back( DataEntry( 5.50, 5.501661, 0.055016 ) );
    M_static_table.push_back( DataEntry( 5.60, 5.612802, 0.056127 ) );
    M_static_table.push_back( DataEntry( 5.70, 5.697415, 0.028488 ) );
    M_static_table.push_back( DataEntry( 5.80, 5.783737, 0.057836 ) );
    M_static_table.push_back( DataEntry( 5.90, 5.900576, 0.059004 ) );
    M_static_table.push_back( DataEntry( 6.00, 6.019776, 0.060197 ) );
    M_static_table.push_back( DataEntry( 6.10, 6.110524, 0.030553 ) );
    M_static_table.push_back( DataEntry( 6.20, 6.203106, 0.062030 ) );
    M_static_table.push_back( DataEntry( 6.30, 6.296617, 0.031483 ) );
    M_static_table.push_back( DataEntry( 6.40, 6.392018, 0.063919 ) );
    M_static_table.push_back( DataEntry( 6.50, 6.488378, 0.032443 ) );
    M_static_table.push_back( DataEntry( 6.60, 6.586684, 0.065865 ) );
    M_static_table.push_back( DataEntry( 6.70, 6.685978, 0.033430 ) );
    M_static_table.push_back( DataEntry( 6.80, 6.787278, 0.067871 ) );
    M_static_table.push_back( DataEntry( 6.90, 6.889597, 0.034449 ) );
    M_static_table.push_back( DataEntry( 7.00, 6.993982, 0.069938 ) );
    M_static_table.push_back( DataEntry( 7.10, 7.099416, 0.035497 ) );
    M_static_table.push_back( DataEntry( 7.20, 7.206980, 0.072068 ) );
    M_static_table.push_back( DataEntry( 7.30, 7.315626, 0.036579 ) );
    M_static_table.push_back( DataEntry( 7.40, 7.389149, 0.036946 ) );
    M_static_table.push_back( DataEntry( 7.50, 7.501102, 0.075009 ) );
    M_static_table.push_back( DataEntry( 7.60, 7.614182, 0.038072 ) );
    M_static_table.push_back( DataEntry( 7.70, 7.690706, 0.038454 ) );
    M_static_table.push_back( DataEntry( 7.80, 7.807228, 0.078070 ) );
    M_static_table.push_back( DataEntry( 7.90, 7.924922, 0.039625 ) );
    M_static_table.push_back( DataEntry( 8.00, 8.004569, 0.040023 ) );
    M_static_table.push_back( DataEntry( 8.10, 8.085016, 0.040425 ) );
    M_static_table.push_back( DataEntry( 8.20, 8.207513, 0.082073 ) );
    M_static_table.push_back( DataEntry( 8.30, 8.331242, 0.041656 ) );
    M_static_table.push_back( DataEntry( 8.40, 8.414972, 0.042075 ) );
    M_static_table.push_back( DataEntry( 8.50, 8.499544, 0.042498 ) );
    M_static_table.push_back( DataEntry( 8.60, 8.584966, 0.042925 ) );
    M_static_table.push_back( DataEntry( 8.70, 8.671246, 0.043356 ) );
    M_static_table.push_back( DataEntry( 8.80, 8.802625, 0.088024 ) );
    M_static_table.push_back( DataEntry( 8.90, 8.935325, 0.044677 ) );
    M_static_table.push_back( DataEntry( 9.00, 9.025126, 0.045125 ) );
    M_static_table.push_back( DataEntry( 9.10, 9.115830, 0.045579 ) );
    M_static_table.push_back( DataEntry( 9.20, 9.207446, 0.046037 ) );
    M_static_table.push_back( DataEntry( 9.30, 9.299982, 0.046500 ) );
    M_static_table.push_back( DataEntry( 9.40, 9.393448, 0.046967 ) );
    M_static_table.push_back( DataEntry( 9.50, 9.487854, 0.047439 ) );
    M_static_table.push_back( DataEntry( 9.60, 9.583209, 0.047916 ) );
    M_static_table.push_back( DataEntry( 9.70, 9.679522, 0.048398 ) );
    M_static_table.push_back( DataEntry( 9.80, 9.776803, 0.048884 ) );
    M_static_table.push_back( DataEntry( 9.90, 9.875061, 0.049375 ) );
    M_static_table.push_back( DataEntry( 10.00, 9.974307, 0.049871 ) );
    M_static_table.push_back( DataEntry( 10.10, 10.074550, 0.050372 ) );
    M_static_table.push_back( DataEntry( 10.20, 10.175801, 0.050879 ) );
    M_static_table.push_back( DataEntry( 10.30, 10.278070, 0.051390 ) );
    M_static_table.push_back( DataEntry( 10.40, 10.381366, 0.051907 ) );
    M_static_table.push_back( DataEntry( 10.50, 10.485700, 0.052428 ) );
    M_static_table.push_back( DataEntry( 10.60, 10.591083, 0.052955 ) );
    M_static_table.push_back( DataEntry( 10.70, 10.697526, 0.053488 ) );
    M_static_table.push_back( DataEntry( 10.80, 10.805038, 0.054025 ) );
    M_static_table.push_back( DataEntry( 10.90, 10.913630, 0.054568 ) );
    M_static_table.push_back( DataEntry( 11.00, 11.023314, 0.055116 ) );
    M_static_table.push_back( DataEntry( 11.10, 11.134100, 0.055670 ) );
    M_static_table.push_back( DataEntry( 11.20, 11.246000, 0.056230 ) );
    M_static_table.push_back( DataEntry( 11.40, 11.359024, 0.056795 ) );
    M_static_table.push_back( DataEntry( 11.50, 11.473184, 0.057366 ) );
    M_static_table.push_back( DataEntry( 11.60, 11.588491, 0.057942 ) );
    M_static_table.push_back( DataEntry( 11.70, 11.704958, 0.058525 ) );
    M_static_table.push_back( DataEntry( 11.80, 11.822595, 0.059113 ) );
    M_static_table.push_back( DataEntry( 11.90, 11.941414, 0.059707 ) );
    M_static_table.push_back( DataEntry( 12.10, 12.061427, 0.060307 ) );
    M_static_table.push_back( DataEntry( 12.20, 12.182646, 0.060913 ) );
    M_static_table.push_back( DataEntry( 12.30, 12.305083, 0.061525 ) );
    M_static_table.push_back( DataEntry( 12.40, 12.428752, 0.062144 ) );
    M_static_table.push_back( DataEntry( 12.60, 12.553663, 0.062768 ) );
    M_static_table.push_back( DataEntry( 12.70, 12.679829, 0.063399 ) );
    M_static_table.push_back( DataEntry( 12.80, 12.807264, 0.064036 ) );
    M_static_table.push_back( DataEntry( 12.90, 12.935979, 0.064680 ) );
    M_static_table.push_back( DataEntry( 13.10, 13.065988, 0.065330 ) );
    M_static_table.push_back( DataEntry( 13.20, 13.197303, 0.065986 ) );
    M_static_table.push_back( DataEntry( 13.30, 13.329938, 0.066649 ) );
    M_static_table.push_back( DataEntry( 13.50, 13.463906, 0.067319 ) );
    M_static_table.push_back( DataEntry( 13.60, 13.599221, 0.067996 ) );
    M_static_table.push_back( DataEntry( 13.70, 13.735895, 0.068679 ) );
    M_static_table.push_back( DataEntry( 13.90, 13.873943, 0.069369 ) );
    M_static_table.push_back( DataEntry( 14.00, 14.013379, 0.070067 ) );
    M_static_table.push_back( DataEntry( 14.20, 14.154216, 0.070771 ) );
    M_static_table.push_back( DataEntry( 14.30, 14.296468, 0.071482 ) );
    M_static_table.push_back( DataEntry( 14.40, 14.440149, 0.072200 ) );
    M_static_table.push_back( DataEntry( 14.60, 14.585275, 0.072926 ) );
    M_static_table.push_back( DataEntry( 14.70, 14.731860, 0.073659 ) );
    M_static_table.push_back( DataEntry( 14.90, 14.879917, 0.074399 ) );
    M_static_table.push_back( DataEntry( 15.00, 15.029463, 0.075147 ) );
    M_static_table.push_back( DataEntry( 15.20, 15.180512, 0.075902 ) );
    M_static_table.push_back( DataEntry( 15.30, 15.333078, 0.076665 ) );
    M_static_table.push_back( DataEntry( 15.50, 15.487178, 0.077435 ) );
    M_static_table.push_back( DataEntry( 15.60, 15.642827, 0.078214 ) );
    M_static_table.push_back( DataEntry( 15.80, 15.800040, 0.079000 ) );
    M_static_table.push_back( DataEntry( 16.00, 15.958833, 0.079794 ) );
    M_static_table.push_back( DataEntry( 16.10, 16.119222, 0.080596 ) );
    M_static_table.push_back( DataEntry( 16.30, 16.281223, 0.081406 ) );
    M_static_table.push_back( DataEntry( 16.40, 16.444852, 0.082224 ) );
    M_static_table.push_back( DataEntry( 16.60, 16.610125, 0.083051 ) );
    M_static_table.push_back( DataEntry( 16.80, 16.777060, 0.083885 ) );
    M_static_table.push_back( DataEntry( 16.90, 16.945672, 0.084729 ) );
    M_static_table.push_back( DataEntry( 17.10, 17.115979, 0.085580 ) );
    M_static_table.push_back( DataEntry( 17.30, 17.287998, 0.086440 ) );
    M_static_table.push_back( DataEntry( 17.50, 17.461745, 0.087309 ) );
    M_static_table.push_back( DataEntry( 17.60, 17.637239, 0.088186 ) );
    M_static_table.push_back( DataEntry( 17.80, 17.814496, 0.089072 ) );
    M_static_table.push_back( DataEntry( 18.00, 17.993534, 0.089968 ) );
    M_static_table.push_back( DataEntry( 18.20, 18.174372, 0.090872 ) );
    M_static_table.push_back( DataEntry( 18.40, 18.357028, 0.091785 ) );
    M_static_table.push_back( DataEntry( 18.50, 18.541519, 0.092708 ) );
    M_static_table.push_back( DataEntry( 18.70, 18.727865, 0.093639 ) );
    M_static_table.push_back( DataEntry( 18.90, 18.916083, 0.094580 ) );
    M_static_table.push_back( DataEntry( 19.10, 19.106192, 0.095531 ) );
    M_static_table.push_back( DataEntry( 19.30, 19.298213, 0.096491 ) );
    M_static_table.push_back( DataEntry( 19.50, 19.492163, 0.097461 ) );
    M_static_table.push_back( DataEntry( 19.70, 19.688063, 0.098440 ) );
    M_static_table.push_back( DataEntry( 19.90, 19.885931, 0.099429 ) );
    M_static_table.push_back( DataEntry( 20.10, 20.085788, 0.100429 ) );
    M_static_table.push_back( DataEntry( 20.30, 20.287653, 0.101438 ) );
    M_static_table.push_back( DataEntry( 20.50, 20.491547, 0.102458 ) );
    M_static_table.push_back( DataEntry( 20.70, 20.697491, 0.103487 ) );
    M_static_table.push_back( DataEntry( 20.90, 20.905504, 0.104528 ) );
    M_static_table.push_back( DataEntry( 21.10, 21.115609, 0.105578 ) );
    M_static_table.push_back( DataEntry( 21.30, 21.327824, 0.106639 ) );
    M_static_table.push_back( DataEntry( 21.50, 21.542172, 0.107711 ) );
    M_static_table.push_back( DataEntry( 21.80, 21.758674, 0.108793 ) );
    M_static_table.push_back( DataEntry( 22.00, 21.977353, 0.109887 ) );
    M_static_table.push_back( DataEntry( 22.20, 22.198229, 0.110991 ) );
    M_static_table.push_back( DataEntry( 22.40, 22.421325, 0.112107 ) );
    M_static_table.push_back( DataEntry( 22.60, 22.646663, 0.113233 ) );
    M_static_table.push_back( DataEntry( 22.90, 22.874266, 0.114371 ) );
    M_static_table.push_back( DataEntry( 23.10, 23.104156, 0.115521 ) );
    M_static_table.push_back( DataEntry( 23.30, 23.336357, 0.116682 ) );
    M_static_table.push_back( DataEntry( 23.60, 23.570891, 0.117854 ) );
    M_static_table.push_back( DataEntry( 23.80, 23.807782, 0.119038 ) );
    M_static_table.push_back( DataEntry( 24.00, 24.047054, 0.120235 ) );
    M_static_table.push_back( DataEntry( 24.30, 24.288731, 0.121443 ) );
    M_static_table.push_back( DataEntry( 24.50, 24.532837, 0.122664 ) );
    M_static_table.push_back( DataEntry( 24.80, 24.779396, 0.123896 ) );
    M_static_table.push_back( DataEntry( 25.00, 25.028433, 0.125142 ) );
    M_static_table.push_back( DataEntry( 25.30, 25.279973, 0.126399 ) );
    M_static_table.push_back( DataEntry( 25.50, 25.534041, 0.127670 ) );
    M_static_table.push_back( DataEntry( 25.80, 25.790663, 0.128953 ) );
    M_static_table.push_back( DataEntry( 26.00, 26.049863, 0.130249 ) );
    M_static_table.push_back( DataEntry( 26.30, 26.311668, 0.131558 ) );
    M_static_table.push_back( DataEntry( 26.60, 26.576105, 0.132880 ) );
    M_static_table.push_back( DataEntry( 26.80, 26.843200, 0.134216 ) );
    M_static_table.push_back( DataEntry( 27.10, 27.112978, 0.135564 ) );
    M_static_table.push_back( DataEntry( 27.40, 27.385468, 0.136927 ) );
    M_static_table.push_back( DataEntry( 27.70, 27.660696, 0.138303 ) );
    M_static_table.push_back( DataEntry( 27.90, 27.938691, 0.139693 ) );
    M_static_table.push_back( DataEntry( 28.20, 28.219480, 0.141097 ) );
    M_static_table.push_back( DataEntry( 28.50, 28.503090, 0.142515 ) );
    M_static_table.push_back( DataEntry( 28.80, 28.789551, 0.143947 ) );
    M_static_table.push_back( DataEntry( 29.10, 29.078891, 0.145394 ) );
    M_static_table.push_back( DataEntry( 29.40, 29.371138, 0.146855 ) );
    M_static_table.push_back( DataEntry( 29.70, 29.666323, 0.148331 ) );
    M_static_table.push_back( DataEntry( 30.00, 29.964475, 0.149822 ) );
    M_static_table.push_back( DataEntry( 30.30, 30.265623, 0.151328 ) );
    M_static_table.push_back( DataEntry( 30.60, 30.569797, 0.152848 ) );
    M_static_table.push_back( DataEntry( 30.90, 30.877029, 0.154385 ) );
    M_static_table.push_back( DataEntry( 31.20, 31.187348, 0.155936 ) );
    M_static_table.push_back( DataEntry( 31.50, 31.500786, 0.157503 ) );
    M_static_table.push_back( DataEntry( 31.80, 31.817374, 0.159086 ) );
    M_static_table.push_back( DataEntry( 32.10, 32.137144, 0.160685 ) );
    M_static_table.push_back( DataEntry( 32.50, 32.460128, 0.162300 ) );
    M_static_table.push_back( DataEntry( 32.80, 32.786358, 0.163930 ) );
    M_static_table.push_back( DataEntry( 33.10, 33.115866, 0.165578 ) );
    M_static_table.push_back( DataEntry( 33.40, 33.448686, 0.167242 ) );
    M_static_table.push_back( DataEntry( 33.80, 33.784851, 0.168923 ) );
    M_static_table.push_back( DataEntry( 34.10, 34.124394, 0.170621 ) );
    M_static_table.push_back( DataEntry( 34.50, 34.467350, 0.172336 ) );
    M_static_table.push_back( DataEntry( 34.80, 34.813753, 0.174067 ) );
    M_static_table.push_back( DataEntry( 35.20, 35.163637, 0.175817 ) );
    M_static_table.push_back( DataEntry( 35.50, 35.517037, 0.177584 ) );
    M_static_table.push_back( DataEntry( 35.90, 35.873989, 0.179369 ) );
    M_static_table.push_back( DataEntry( 36.20, 36.234529, 0.181171 ) );
    M_static_table.push_back( DataEntry( 36.60, 36.598691, 0.182992 ) );
    M_static_table.push_back( DataEntry( 37.00, 36.966514, 0.184831 ) );
    M_static_table.push_back( DataEntry( 37.30, 37.338034, 0.186689 ) );
    M_static_table.push_back( DataEntry( 37.70, 37.713288, 0.188565 ) );
    M_static_table.push_back( DataEntry( 38.10, 38.092312, 0.190460 ) );
    M_static_table.push_back( DataEntry( 38.50, 38.475147, 0.192375 ) );
    M_static_table.push_back( DataEntry( 38.90, 38.861829, 0.194308 ) );
    M_static_table.push_back( DataEntry( 39.30, 39.252396, 0.196260 ) );
    M_static_table.push_back( DataEntry( 39.60, 39.646889, 0.198233 ) );
    M_static_table.push_back( DataEntry( 40.00, 40.045347, 0.200225 ) );
    M_static_table.push_back( DataEntry( 40.40, 40.447810, 0.202238 ) );
    M_static_table.push_back( DataEntry( 40.90, 40.854317, 0.204270 ) );
    M_static_table.push_back( DataEntry( 41.30, 41.264910, 0.206323 ) );
    M_static_table.push_back( DataEntry( 41.70, 41.679629, 0.208397 ) );
    M_static_table.push_back( DataEntry( 42.10, 42.098516, 0.210491 ) );
    M_static_table.push_back( DataEntry( 42.50, 42.521613, 0.212606 ) );
    M_static_table.push_back( DataEntry( 42.90, 42.948962, 0.214743 ) );
    M_static_table.push_back( DataEntry( 43.40, 43.380607, 0.216902 ) );
    M_static_table.push_back( DataEntry( 43.80, 43.816589, 0.219081 ) );
    M_static_table.push_back( DataEntry( 44.30, 44.256953, 0.221283 ) );
    M_static_table.push_back( DataEntry( 44.70, 44.701743, 0.223507 ) );
    M_static_table.push_back( DataEntry( 45.20, 45.151003, 0.225753 ) );
    M_static_table.push_back( DataEntry( 45.60, 45.604778, 0.228022 ) );
    M_static_table.push_back( DataEntry( 46.10, 46.063114, 0.230314 ) );
    M_static_table.push_back( DataEntry( 46.50, 46.526056, 0.232629 ) );
    M_static_table.push_back( DataEntry( 47.00, 46.993650, 0.234966 ) );
    M_static_table.push_back( DataEntry( 47.50, 47.465944, 0.237328 ) );
    M_static_table.push_back( DataEntry( 47.90, 47.942985, 0.239713 ) );
    M_static_table.push_back( DataEntry( 48.40, 48.424820, 0.242122 ) );
    M_static_table.push_back( DataEntry( 48.90, 48.911498, 0.244556 ) );
    M_static_table.push_back( DataEntry( 49.40, 49.403066, 0.247013 ) );
    M_static_table.push_back( DataEntry( 49.90, 49.899575, 0.249496 ) );
    M_static_table.push_back( DataEntry( 50.40, 50.401075, 0.252004 ) );
    M_static_table.push_back( DataEntry( 50.90, 50.907614, 0.254536 ) );
    M_static_table.push_back( DataEntry( 51.40, 51.419244, 0.257095 ) );
    M_static_table.push_back( DataEntry( 51.90, 51.936016, 0.259678 ) );
    M_static_table.push_back( DataEntry( 52.50, 52.457981, 0.262288 ) );
    M_static_table.push_back( DataEntry( 53.00, 52.985193, 0.264924 ) );
    M_static_table.push_back( DataEntry( 53.50, 53.517703, 0.267587 ) );
    M_static_table.push_back( DataEntry( 54.10, 54.055565, 0.270276 ) );
    M_static_table.push_back( DataEntry( 54.60, 54.598832, 0.272992 ) );
    M_static_table.push_back( DataEntry( 55.10, 55.147560, 0.275736 ) );
    M_static_table.push_back( DataEntry( 55.70, 55.701802, 0.278507 ) );
    M_static_table.push_back( DataEntry( 56.30, 56.261614, 0.281306 ) );
    M_static_table.push_back( DataEntry( 56.80, 56.827053, 0.284133 ) );
    M_static_table.push_back( DataEntry( 57.40, 57.398175, 0.286989 ) );
    M_static_table.push_back( DataEntry( 58.00, 57.975036, 0.289873 ) );
    M_static_table.push_back( DataEntry( 58.60, 58.557694, 0.292786 ) );
    M_static_table.push_back( DataEntry( 59.10, 59.146209, 0.295729 ) );
    M_static_table.push_back( DataEntry( 59.70, 59.740638, 0.298701 ) );
    M_static_table.push_back( DataEntry( 60.30, 60.341042, 0.301703 ) );
    M_static_table.push_back( DataEntry( 60.90, 60.947479, 0.304735 ) );
    M_static_table.push_back( DataEntry( 61.60, 61.560012, 0.307798 ) );
    M_static_table.push_back( DataEntry( 62.20, 62.178700, 0.310891 ) );
    M_static_table.push_back( DataEntry( 62.80, 62.803606, 0.314015 ) );
    M_static_table.push_back( DataEntry( 63.40, 63.434793, 0.317172 ) );
    M_static_table.push_back( DataEntry( 64.10, 64.072323, 0.320359 ) );
    M_static_table.push_back( DataEntry( 64.70, 64.716261, 0.323579 ) );
    M_static_table.push_back( DataEntry( 65.40, 65.366670, 0.326831 ) );
    M_static_table.push_back( DataEntry( 66.00, 66.023616, 0.330116 ) );
    M_static_table.push_back( DataEntry( 66.70, 66.687164, 0.333433 ) );
    M_static_table.push_back( DataEntry( 67.40, 67.357381, 0.336784 ) );
    M_static_table.push_back( DataEntry( 68.00, 68.034334, 0.340169 ) );
    M_static_table.push_back( DataEntry( 68.70, 68.718091, 0.343588 ) );
    M_static_table.push_back( DataEntry( 69.40, 69.408719, 0.347041 ) );
    M_static_table.push_back( DataEntry( 70.10, 70.106289, 0.350529 ) );
    M_static_table.push_back( DataEntry( 70.80, 70.810869, 0.354052 ) );
    M_static_table.push_back( DataEntry( 71.50, 71.522530, 0.357610 ) );
    M_static_table.push_back( DataEntry( 72.20, 72.241343, 0.361204 ) );
    M_static_table.push_back( DataEntry( 73.00, 72.967380, 0.364834 ) );
    M_static_table.push_back( DataEntry( 73.70, 73.700715, 0.368501 ) );
    M_static_table.push_back( DataEntry( 74.40, 74.441419, 0.372204 ) );
    M_static_table.push_back( DataEntry( 75.20, 75.189568, 0.375945 ) );
    M_static_table.push_back( DataEntry( 75.90, 75.945235, 0.379723 ) );
    M_static_table.push_back( DataEntry( 76.70, 76.708498, 0.383540 ) );
    M_static_table.push_back( DataEntry( 77.50, 77.479431, 0.387394 ) );
    M_static_table.push_back( DataEntry( 78.30, 78.258113, 0.391288 ) );
    M_static_table.push_back( DataEntry( 79.00, 79.044620, 0.395220 ) );
    M_static_table.push_back( DataEntry( 79.80, 79.839031, 0.399192 ) );
    M_static_table.push_back( DataEntry( 80.60, 80.641427, 0.403204 ) );
    M_static_table.push_back( DataEntry( 81.50, 81.451886, 0.407256 ) );
    M_static_table.push_back( DataEntry( 82.30, 82.270492, 0.411350 ) );
    M_static_table.push_back( DataEntry( 83.10, 83.097324, 0.415483 ) );
    M_static_table.push_back( DataEntry( 83.90, 83.932466, 0.419659 ) );
    M_static_table.push_back( DataEntry( 84.80, 84.776001, 0.423876 ) );
    M_static_table.push_back( DataEntry( 85.60, 85.628014, 0.428137 ) );
    M_static_table.push_back( DataEntry( 86.50, 86.488590, 0.432440 ) );
    M_static_table.push_back( DataEntry( 87.40, 87.357815, 0.436786 ) );
    M_static_table.push_back( DataEntry( 88.20, 88.235775, 0.441175 ) );
    M_static_table.push_back( DataEntry( 89.10, 89.122560, 0.445610 ) );
    M_static_table.push_back( DataEntry( 90.00, 90.018257, 0.450088 ) );
    M_static_table.push_back( DataEntry( 90.90, 90.922955, 0.454611 ) );
    M_static_table.push_back( DataEntry( 91.80, 91.836746, 0.459180 ) );
    M_static_table.push_back( DataEntry( 92.80, 92.759720, 0.463795 ) );
    M_static_table.push_back( DataEntry( 93.70, 93.691971, 0.468456 ) );
    M_static_table.push_back( DataEntry( 94.60, 94.633591, 0.473164 ) );
    M_static_table.push_back( DataEntry( 95.60, 95.584675, 0.477920 ) );
    M_static_table.push_back( DataEntry( 96.50, 96.545317, 0.482723 ) );
    M_static_table.push_back( DataEntry( 97.50, 97.515613, 0.487574 ) );
    M_static_table.push_back( DataEntry( 98.50, 98.495661, 0.492474 ) );
    M_static_table.push_back( DataEntry( 99.50, 99.485559, 0.497424 ) );
    M_static_table.push_back( DataEntry( 100.50, 100.485406, 0.502423 ) );
    M_static_table.push_back( DataEntry( 101.50, 101.495301, 0.507473 ) );
    M_static_table.push_back( DataEntry( 102.50, 102.515346, 0.512573 ) );
    M_static_table.push_back( DataEntry( 103.50, 103.545642, 0.517724 ) );
    M_static_table.push_back( DataEntry( 104.60, 104.586293, 0.522928 ) );
    M_static_table.push_back( DataEntry( 105.60, 105.637403, 0.528183 ) );
    M_static_table.push_back( DataEntry( 106.70, 106.699076, 0.533491 ) );
    M_static_table.push_back( DataEntry( 107.80, 107.771420, 0.538853 ) );
    M_static_table.push_back( DataEntry( 108.90, 108.854540, 0.544268 ) );
    M_static_table.push_back( DataEntry( 109.90, 109.948547, 0.549739 ) );
    M_static_table.push_back( DataEntry( 111.10, 111.053548, 0.555263 ) );
    M_static_table.push_back( DataEntry( 112.20, 112.169655, 0.560844 ) );
    M_static_table.push_back( DataEntry( 113.30, 113.296978, 0.566480 ) );
    M_static_table.push_back( DataEntry( 114.40, 114.435632, 0.572174 ) );
    M_static_table.push_back( DataEntry( 115.60, 115.585729, 0.577924 ) );
    M_static_table.push_back( DataEntry( 116.70, 116.747385, 0.583732 ) );
    M_static_table.push_back( DataEntry( 117.90, 117.920716, 0.589599 ) );
    M_static_table.push_back( DataEntry( 119.10, 119.105839, 0.595525 ) );
    M_static_table.push_back( DataEntry( 120.30, 120.302872, 0.601509 ) );
    M_static_table.push_back( DataEntry( 121.50, 121.511936, 0.607555 ) );
    M_static_table.push_back( DataEntry( 122.70, 122.733152, 0.613661 ) );
    M_static_table.push_back( DataEntry( 124.00, 123.966640, 0.619828 ) );
    M_static_table.push_back( DataEntry( 125.20, 125.212526, 0.626058 ) );
    M_static_table.push_back( DataEntry( 126.50, 126.470933, 0.632350 ) );
    M_static_table.push_back( DataEntry( 127.70, 127.741987, 0.638705 ) );
    M_static_table.push_back( DataEntry( 129.00, 129.025815, 0.645124 ) );
    M_static_table.push_back( DataEntry( 130.30, 130.322546, 0.651608 ) );
    M_static_table.push_back( DataEntry( 131.60, 131.632309, 0.658156 ) );
    M_static_table.push_back( DataEntry( 133.00, 132.955236, 0.664771 ) );
    M_static_table.push_back( DataEntry( 134.30, 134.291458, 0.671452 ) );
    M_static_table.push_back( DataEntry( 135.60, 135.641110, 0.678200 ) );
    M_static_table.push_back( DataEntry( 137.00, 137.004326, 0.685016 ) );
    M_static_table.push_back( DataEntry( 138.40, 138.381242, 0.691901 ) );
    M_static_table.push_back( DataEntry( 139.80, 139.771997, 0.698855 ) );
    M_static_table.push_back( DataEntry( 141.20, 141.176729, 0.705878 ) );
    M_static_table.push_back( DataEntry( 142.60, 142.595578, 0.712972 ) );
    M_static_table.push_back( DataEntry( 144.00, 144.028688, 0.720138 ) );
    M_static_table.push_back( DataEntry( 145.50, 145.476200, 0.727375 ) );
    M_static_table.push_back( DataEntry( 146.90, 146.938260, 0.734685 ) );
    M_static_table.push_back( DataEntry( 148.40, 148.415014, 0.742069 ) );
    M_static_table.push_back( DataEntry( 149.90, 149.906610, 0.749527 ) );
    M_static_table.push_back( DataEntry( 151.40, 151.413197, 0.757060 ) );
    M_static_table.push_back( DataEntry( 152.90, 152.934924, 0.764668 ) );
    M_static_table.push_back( DataEntry( 154.50, 154.471946, 0.772354 ) );
    M_static_table.push_back( DataEntry( 156.00, 156.024415, 0.780116 ) );
    M_static_table.push_back( DataEntry( 157.60, 157.592486, 0.787956 ) );
    M_static_table.push_back( DataEntry( 159.20, 159.176317, 0.795875 ) );
    M_static_table.push_back( DataEntry( 160.80, 160.776066, 0.803874 ) );
    M_static_table.push_back( DataEntry( 162.40, 162.391892, 0.811953 ) );
    M_static_table.push_back( DataEntry( 164.00, 164.023958, 0.820113 ) );
    M_static_table.push_back( DataEntry( 165.70, 165.672426, 0.828355 ) );
    M_static_table.push_back( DataEntry( 167.30, 167.337461, 0.836680 ) );
    M_static_table.push_back( DataEntry( 169.00, 169.019231, 0.845089 ) );
    M_static_table.push_back( DataEntry( 170.70, 170.717902, 0.853582 ) );
    M_static_table.push_back( DataEntry( 172.40, 172.433646, 0.862161 ) );
    M_static_table.push_back( DataEntry( 174.20, 174.166633, 0.870826 ) );
    M_static_table.push_back( DataEntry( 175.90, 175.917036, 0.879578 ) );
    M_static_table.push_back( DataEntry( 177.70, 177.685032, 0.888418 ) );
    M_static_table.push_back( DataEntry( 179.50, 179.470796, 0.897347 ) );


    M_movable_table.push_back( DataEntry( 0.00, 0.026170, 0.026170 ) );
    M_movable_table.push_back( DataEntry( 0.10, 0.104789, 0.052450 ) );
    M_movable_table.push_back( DataEntry( 0.20, 0.208239, 0.051002 ) );
    M_movable_table.push_back( DataEntry( 0.30, 0.304589, 0.045349 ) );
    M_movable_table.push_back( DataEntry( 0.40, 0.411152, 0.061215 ) );
    M_movable_table.push_back( DataEntry( 0.50, 0.524658, 0.052292 ) );
    M_movable_table.push_back( DataEntry( 0.60, 0.607289, 0.030340 ) );
    M_movable_table.push_back( DataEntry( 0.70, 0.708214, 0.070587 ) );
    M_movable_table.push_back( DataEntry( 0.80, 0.819754, 0.040954 ) );
    M_movable_table.push_back( DataEntry( 0.90, 0.905969, 0.045262 ) );
    M_movable_table.push_back( DataEntry( 1.00, 1.001251, 0.050021 ) );
    M_movable_table.push_back( DataEntry( 1.10, 1.106553, 0.055282 ) );
    M_movable_table.push_back( DataEntry( 1.20, 1.222930, 0.061096 ) );
    M_movable_table.push_back( DataEntry( 1.30, 1.351546, 0.067521 ) );
    M_movable_table.push_back( DataEntry( 1.50, 1.493690, 0.074623 ) );
    M_movable_table.push_back( DataEntry( 1.60, 1.650783, 0.082471 ) );
    M_movable_table.push_back( DataEntry( 1.80, 1.824397, 0.091144 ) );
    M_movable_table.push_back( DataEntry( 2.00, 2.016270, 0.100731 ) );
    M_movable_table.push_back( DataEntry( 2.20, 2.228323, 0.111324 ) );
    M_movable_table.push_back( DataEntry( 2.50, 2.462678, 0.123032 ) );
    M_movable_table.push_back( DataEntry( 2.70, 2.721681, 0.135972 ) );
    M_movable_table.push_back( DataEntry( 3.00, 3.007922, 0.150271 ) );
    M_movable_table.push_back( DataEntry( 3.30, 3.324268, 0.166076 ) );
    M_movable_table.push_back( DataEntry( 3.70, 3.673884, 0.183542 ) );
    M_movable_table.push_back( DataEntry( 4.10, 4.060270, 0.202845 ) );
    M_movable_table.push_back( DataEntry( 4.50, 4.487293, 0.224179 ) );
    M_movable_table.push_back( DataEntry( 5.00, 4.959225, 0.247755 ) );
    M_movable_table.push_back( DataEntry( 5.50, 5.480791, 0.273812 ) );
    M_movable_table.push_back( DataEntry( 6.00, 6.057211, 0.302609 ) );
    M_movable_table.push_back( DataEntry( 6.70, 6.694254, 0.334435 ) );
    M_movable_table.push_back( DataEntry( 7.40, 7.398295, 0.369608 ) );
    M_movable_table.push_back( DataEntry( 8.20, 8.176380, 0.408479 ) );
    M_movable_table.push_back( DataEntry( 9.00, 9.036297, 0.451439 ) );
    M_movable_table.push_back( DataEntry( 10.00, 9.986652, 0.498917 ) );
    M_movable_table.push_back( DataEntry( 11.00, 11.036958, 0.551389 ) );
    M_movable_table.push_back( DataEntry( 12.20, 12.197725, 0.609379 ) );
    M_movable_table.push_back( DataEntry( 13.50, 13.480571, 0.673468 ) );
    M_movable_table.push_back( DataEntry( 14.90, 14.898335, 0.744297 ) );
    M_movable_table.push_back( DataEntry( 16.40, 16.465206, 0.822576 ) );
    M_movable_table.push_back( DataEntry( 18.20, 18.196867, 0.909087 ) );
    M_movable_table.push_back( DataEntry( 20.10, 20.110649, 1.004696 ) );
    M_movable_table.push_back( DataEntry( 22.20, 22.225705, 1.110361 ) );
    M_movable_table.push_back( DataEntry( 24.50, 24.563202, 1.227138 ) );
    M_movable_table.push_back( DataEntry( 27.10, 27.146537, 1.356198 ) );
    M_movable_table.push_back( DataEntry( 30.00, 30.001563, 1.498830 ) );
    M_movable_table.push_back( DataEntry( 33.10, 33.156855, 1.656463 ) );
    M_movable_table.push_back( DataEntry( 36.60, 36.643992, 1.830675 ) );
    M_movable_table.push_back( DataEntry( 40.40, 40.497874, 2.023208 ) );
    M_movable_table.push_back( DataEntry( 44.70, 44.757073, 2.235991 ) );
    M_movable_table.push_back( DataEntry( 49.40, 49.464215, 2.471152 ) );
    M_movable_table.push_back( DataEntry( 54.60, 54.666412, 2.731046 ) );
    M_movable_table.push_back( DataEntry( 60.30, 60.415729, 3.018272 ) );
    M_movable_table.push_back( DataEntry( 66.70, 66.769706, 3.335706 ) );
    M_movable_table.push_back( DataEntry( 73.70, 73.791938, 3.686526 ) );
    M_movable_table.push_back( DataEntry( 81.50, 81.552704, 4.074241 ) );
    M_movable_table.push_back( DataEntry( 90.00, 90.129676, 4.502732 ) );
    M_movable_table.push_back( DataEntry( 99.50, 99.608697, 4.976289 ) );
    M_movable_table.push_back( DataEntry( 109.90, 110.084635, 5.499650 ) );
    M_movable_table.push_back( DataEntry( 121.50, 121.662337, 6.078053 ) );
    M_movable_table.push_back( DataEntry( 134.30, 134.457677, 6.717287 ) );
    M_movable_table.push_back( DataEntry( 148.40, 148.598714, 7.423750 ) );
    M_movable_table.push_back( DataEntry( 164.00, 163.226977, 8.204513 ) );
    M_movable_table.push_back( DataEntry( 181.30, 181.498879, 9.067389 ) );

    // sort(M_static_table.begin(), M_static_table.end( ) );
    // sort(M_movable_table.begin(), M_dunamic_table.end( ) );
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
ObjectTable::getStaticObjInfo( const double & see_dist,
                               double * ave,
                               double * err ) const
{
    std::vector< DataEntry >::const_iterator
        it = std::lower_bound( M_static_table.begin(),
                               M_static_table.end(),
                               ObjectTable::DataEntry( see_dist - 0.001 ),
                               DataEntryCmp() );
    if ( it == M_static_table.end() )
    {
        std::cerr << "ObjectTable::getStaticObjInfo : illegal dist : "
                  << see_dist << std::endl;
        return false;
    }

    *ave = it->M_average;
    *err = it->M_error;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
ObjectTable::getMovableObjInfo( const double & see_dist,
                                double * ave,
                                double * err ) const
{

    std::vector< DataEntry >::const_iterator
        it = std::lower_bound( M_movable_table.begin(),
                               M_movable_table.end(),
                               DataEntry( see_dist - 0.001 ),
                               DataEntryCmp() );
    if ( it == M_movable_table.end() )
    {
        std::cerr << "ObjectTable::getMovableObjInfo : illegal dist : "
                  << see_dist << std::endl;
        return false;
    }

    *ave = it->M_average;
    *err = it->M_error;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
ObjectTable::create( const double & static_qstep,
                     const double & movable_qstep )
{
    createTable( static_qstep, M_static_table );
    createTable( movable_qstep, M_movable_table );
}


/*-------------------------------------------------------------------*/
/*!

 */
double
ObjectTable::quantize( const double & value,
                       const double & qstep )
{
    return rint( value / qstep ) * qstep;
}

/*-------------------------------------------------------------------*/
/*!

 */
double
ObjectTable::quantize_dist( const double & unq_dist,
                            const double & qstep )
{
    return quantize( std::exp
                     ( quantize( std::log
                                 ( unq_dist + SERVER_EPS ), qstep ) ),
                     0.1);
}

/*-------------------------------------------------------------------*/
/*!

 */
void
ObjectTable::createTable( const double & qstep,
                          std::vector< DataEntry > & table )
{
    table.clear();

    double prev_val = -0.1;

    for ( double dist = 0.0; dist < 180.0; dist += 0.01 )
    {

        double see_dist = quantize_dist( dist, qstep );

        if ( std::fabs( prev_val - see_dist ) < 0.001 )
        {
            continue;
        }
        prev_val = see_dist;

        double min_dist, max_dist;

        // unquantize min
        min_dist = see_dist - 0.05;
        if ( min_dist < SERVER_EPS ) min_dist = SERVER_EPS;
        min_dist = std::log( min_dist );
        min_dist = ( rint( min_dist / qstep ) - 0.5 ) * qstep;
        min_dist = std::exp( min_dist ) - SERVER_EPS;
        if ( min_dist < 0.0 ) min_dist = 0.0;

        // unquantize max
        max_dist = see_dist + 0.049999;
        max_dist = std::log( max_dist );
        max_dist = ( rint( max_dist / qstep ) + 0.49999 ) * qstep;
        max_dist = std::exp( max_dist ) - SERVER_EPS;

        //double ave_dist = (max_dist + min_dist) * 0.5;
        //double err_dist = (max_dist - min_dist) * 0.5;

        table.push_back( DataEntry( see_dist, // quantized dist
                                   (max_dist + min_dist) * 0.5,   // average
                                   (max_dist - min_dist) * 0.5 ) ); // error
    }
}

}
