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
    M_static_table_v18_narrow.clear();
    M_static_table_v18_normal.clear();
    M_static_table_v18_wide.clear();

    M_movable_table.clear();
    M_movable_table_v18_narrow.clear();
    M_movable_table_v18_normal.clear();
    M_movable_table_v18_wide.clear();

    M_static_table.emplace_back( 0.00, 0.025019, 0.025019 );
    M_static_table.emplace_back( 0.10, 0.100178, 0.050142 );
    M_static_table.emplace_back( 0.20, 0.200321, 0.050003 );
    M_static_table.emplace_back( 0.30, 0.301007, 0.050684 );
    M_static_table.emplace_back( 0.40, 0.401636, 0.049945 );
    M_static_table.emplace_back( 0.50, 0.501572, 0.049991 );
    M_static_table.emplace_back( 0.60, 0.599413, 0.047852 );
    M_static_table.emplace_back( 0.70, 0.699639, 0.052376 );
    M_static_table.emplace_back( 0.80, 0.799954, 0.047940 );
    M_static_table.emplace_back( 0.90, 0.897190, 0.049297 );
    M_static_table.emplace_back( 1.00, 0.996257, 0.049771 );
    M_static_table.emplace_back( 1.10, 1.095282, 0.049255 );
    M_static_table.emplace_back( 1.20, 1.198429, 0.053893 );
    M_static_table.emplace_back( 1.30, 1.304474, 0.052152 );
    M_static_table.emplace_back( 1.40, 1.405808, 0.049183 );
    M_static_table.emplace_back( 1.50, 1.499977, 0.044986 );
    M_static_table.emplace_back( 1.60, 1.600974, 0.056011 );
    M_static_table.emplace_back( 1.70, 1.699463, 0.042478 );
    M_static_table.emplace_back( 1.80, 1.795798, 0.053858 );
    M_static_table.emplace_back( 1.90, 1.897073, 0.047417 );
    M_static_table.emplace_back( 2.00, 1.994338, 0.049849 );
    M_static_table.emplace_back( 2.10, 2.096590, 0.052405 );
    M_static_table.emplace_back( 2.20, 2.204085, 0.055091 );
    M_static_table.emplace_back( 2.30, 2.305275, 0.046100 );
    M_static_table.emplace_back( 2.40, 2.399355, 0.047981 );
    M_static_table.emplace_back( 2.50, 2.497274, 0.049940 );
    M_static_table.emplace_back( 2.60, 2.599191, 0.051978 );
    M_static_table.emplace_back( 2.70, 2.705266, 0.054099 );
    M_static_table.emplace_back( 2.80, 2.801381, 0.042018 );
    M_static_table.emplace_back( 2.90, 2.901419, 0.058021 );
    M_static_table.emplace_back( 3.00, 3.004504, 0.045065 );
    M_static_table.emplace_back( 3.10, 3.096005, 0.046437 );
    M_static_table.emplace_back( 3.20, 3.190292, 0.047851 );
    M_static_table.emplace_back( 3.30, 3.287451, 0.049309 );
    M_static_table.emplace_back( 3.40, 3.387569, 0.050810 );
    M_static_table.emplace_back( 3.50, 3.490736, 0.052358 );
    M_static_table.emplace_back( 3.60, 3.597044, 0.053952 );
    M_static_table.emplace_back( 3.70, 3.706591, 0.055596 );
    M_static_table.emplace_back( 3.80, 3.800186, 0.038001 );
    M_static_table.emplace_back( 3.90, 3.896632, 0.058446 );
    M_static_table.emplace_back( 4.00, 3.995026, 0.039950 );
    M_static_table.emplace_back( 4.10, 4.096416, 0.061442 );
    M_static_table.emplace_back( 4.20, 4.199855, 0.041998 );
    M_static_table.emplace_back( 4.30, 4.306444, 0.064592 );
    M_static_table.emplace_back( 4.40, 4.415186, 0.044151 );
    M_static_table.emplace_back( 4.50, 4.504379, 0.045043 );
    M_static_table.emplace_back( 4.60, 4.595374, 0.045953 );
    M_static_table.emplace_back( 4.70, 4.688206, 0.046881 );
    M_static_table.emplace_back( 4.80, 4.782914, 0.047828 );
    M_static_table.emplace_back( 4.90, 4.879536, 0.048795 );
    M_static_table.emplace_back( 5.00, 4.978109, 0.049780 );
    M_static_table.emplace_back( 5.10, 5.078673, 0.050786 );
    M_static_table.emplace_back( 5.20, 5.181269, 0.051812 );
    M_static_table.emplace_back( 5.30, 5.285938, 0.052859 );
    M_static_table.emplace_back( 5.40, 5.392721, 0.053926 );
    M_static_table.emplace_back( 5.50, 5.501661, 0.055016 );
    M_static_table.emplace_back( 5.60, 5.612802, 0.056127 );
    M_static_table.emplace_back( 5.70, 5.697415, 0.028488 );
    M_static_table.emplace_back( 5.80, 5.783737, 0.057836 );
    M_static_table.emplace_back( 5.90, 5.900576, 0.059004 );
    M_static_table.emplace_back( 6.00, 6.019776, 0.060197 );
    M_static_table.emplace_back( 6.10, 6.110524, 0.030553 );
    M_static_table.emplace_back( 6.20, 6.203106, 0.062030 );
    M_static_table.emplace_back( 6.30, 6.296617, 0.031483 );
    M_static_table.emplace_back( 6.40, 6.392018, 0.063919 );
    M_static_table.emplace_back( 6.50, 6.488378, 0.032443 );
    M_static_table.emplace_back( 6.60, 6.586684, 0.065865 );
    M_static_table.emplace_back( 6.70, 6.685978, 0.033430 );
    M_static_table.emplace_back( 6.80, 6.787278, 0.067871 );
    M_static_table.emplace_back( 6.90, 6.889597, 0.034449 );
    M_static_table.emplace_back( 7.00, 6.993982, 0.069938 );
    M_static_table.emplace_back( 7.10, 7.099416, 0.035497 );
    M_static_table.emplace_back( 7.20, 7.206980, 0.072068 );
    M_static_table.emplace_back( 7.30, 7.315626, 0.036579 );
    M_static_table.emplace_back( 7.40, 7.389149, 0.036946 );
    M_static_table.emplace_back( 7.50, 7.501102, 0.075009 );
    M_static_table.emplace_back( 7.60, 7.614182, 0.038072 );
    M_static_table.emplace_back( 7.70, 7.690706, 0.038454 );
    M_static_table.emplace_back( 7.80, 7.807228, 0.078070 );
    M_static_table.emplace_back( 7.90, 7.924922, 0.039625 );
    M_static_table.emplace_back( 8.00, 8.004569, 0.040023 );
    M_static_table.emplace_back( 8.10, 8.085016, 0.040425 );
    M_static_table.emplace_back( 8.20, 8.207513, 0.082073 );
    M_static_table.emplace_back( 8.30, 8.331242, 0.041656 );
    M_static_table.emplace_back( 8.40, 8.414972, 0.042075 );
    M_static_table.emplace_back( 8.50, 8.499544, 0.042498 );
    M_static_table.emplace_back( 8.60, 8.584966, 0.042925 );
    M_static_table.emplace_back( 8.70, 8.671246, 0.043356 );
    M_static_table.emplace_back( 8.80, 8.802625, 0.088024 );
    M_static_table.emplace_back( 8.90, 8.935325, 0.044677 );
    M_static_table.emplace_back( 9.00, 9.025126, 0.045125 );
    M_static_table.emplace_back( 9.10, 9.115830, 0.045579 );
    M_static_table.emplace_back( 9.20, 9.207446, 0.046037 );
    M_static_table.emplace_back( 9.30, 9.299982, 0.046500 );
    M_static_table.emplace_back( 9.40, 9.393448, 0.046967 );
    M_static_table.emplace_back( 9.50, 9.487854, 0.047439 );
    M_static_table.emplace_back( 9.60, 9.583209, 0.047916 );
    M_static_table.emplace_back( 9.70, 9.679522, 0.048398 );
    M_static_table.emplace_back( 9.80, 9.776803, 0.048884 );
    M_static_table.emplace_back( 9.90, 9.875061, 0.049375 );
    M_static_table.emplace_back( 10.00, 9.974307, 0.049871 );
    M_static_table.emplace_back( 10.10, 10.074550, 0.050372 );
    M_static_table.emplace_back( 10.20, 10.175801, 0.050879 );
    M_static_table.emplace_back( 10.30, 10.278070, 0.051390 );
    M_static_table.emplace_back( 10.40, 10.381366, 0.051907 );
    M_static_table.emplace_back( 10.50, 10.485700, 0.052428 );
    M_static_table.emplace_back( 10.60, 10.591083, 0.052955 );
    M_static_table.emplace_back( 10.70, 10.697526, 0.053488 );
    M_static_table.emplace_back( 10.80, 10.805038, 0.054025 );
    M_static_table.emplace_back( 10.90, 10.913630, 0.054568 );
    M_static_table.emplace_back( 11.00, 11.023314, 0.055116 );
    M_static_table.emplace_back( 11.10, 11.134100, 0.055670 );
    M_static_table.emplace_back( 11.20, 11.246000, 0.056230 );
    M_static_table.emplace_back( 11.40, 11.359024, 0.056795 );
    M_static_table.emplace_back( 11.50, 11.473184, 0.057366 );
    M_static_table.emplace_back( 11.60, 11.588491, 0.057942 );
    M_static_table.emplace_back( 11.70, 11.704958, 0.058525 );
    M_static_table.emplace_back( 11.80, 11.822595, 0.059113 );
    M_static_table.emplace_back( 11.90, 11.941414, 0.059707 );
    M_static_table.emplace_back( 12.10, 12.061427, 0.060307 );
    M_static_table.emplace_back( 12.20, 12.182646, 0.060913 );
    M_static_table.emplace_back( 12.30, 12.305083, 0.061525 );
    M_static_table.emplace_back( 12.40, 12.428752, 0.062144 );
    M_static_table.emplace_back( 12.60, 12.553663, 0.062768 );
    M_static_table.emplace_back( 12.70, 12.679829, 0.063399 );
    M_static_table.emplace_back( 12.80, 12.807264, 0.064036 );
    M_static_table.emplace_back( 12.90, 12.935979, 0.064680 );
    M_static_table.emplace_back( 13.10, 13.065988, 0.065330 );
    M_static_table.emplace_back( 13.20, 13.197303, 0.065986 );
    M_static_table.emplace_back( 13.30, 13.329938, 0.066649 );
    M_static_table.emplace_back( 13.50, 13.463906, 0.067319 );
    M_static_table.emplace_back( 13.60, 13.599221, 0.067996 );
    M_static_table.emplace_back( 13.70, 13.735895, 0.068679 );
    M_static_table.emplace_back( 13.90, 13.873943, 0.069369 );
    M_static_table.emplace_back( 14.00, 14.013379, 0.070067 );
    M_static_table.emplace_back( 14.20, 14.154216, 0.070771 );
    M_static_table.emplace_back( 14.30, 14.296468, 0.071482 );
    M_static_table.emplace_back( 14.40, 14.440149, 0.072200 );
    M_static_table.emplace_back( 14.60, 14.585275, 0.072926 );
    M_static_table.emplace_back( 14.70, 14.731860, 0.073659 );
    M_static_table.emplace_back( 14.90, 14.879917, 0.074399 );
    M_static_table.emplace_back( 15.00, 15.029463, 0.075147 );
    M_static_table.emplace_back( 15.20, 15.180512, 0.075902 );
    M_static_table.emplace_back( 15.30, 15.333078, 0.076665 );
    M_static_table.emplace_back( 15.50, 15.487178, 0.077435 );
    M_static_table.emplace_back( 15.60, 15.642827, 0.078214 );
    M_static_table.emplace_back( 15.80, 15.800040, 0.079000 );
    M_static_table.emplace_back( 16.00, 15.958833, 0.079794 );
    M_static_table.emplace_back( 16.10, 16.119222, 0.080596 );
    M_static_table.emplace_back( 16.30, 16.281223, 0.081406 );
    M_static_table.emplace_back( 16.40, 16.444852, 0.082224 );
    M_static_table.emplace_back( 16.60, 16.610125, 0.083051 );
    M_static_table.emplace_back( 16.80, 16.777060, 0.083885 );
    M_static_table.emplace_back( 16.90, 16.945672, 0.084729 );
    M_static_table.emplace_back( 17.10, 17.115979, 0.085580 );
    M_static_table.emplace_back( 17.30, 17.287998, 0.086440 );
    M_static_table.emplace_back( 17.50, 17.461745, 0.087309 );
    M_static_table.emplace_back( 17.60, 17.637239, 0.088186 );
    M_static_table.emplace_back( 17.80, 17.814496, 0.089072 );
    M_static_table.emplace_back( 18.00, 17.993534, 0.089968 );
    M_static_table.emplace_back( 18.20, 18.174372, 0.090872 );
    M_static_table.emplace_back( 18.40, 18.357028, 0.091785 );
    M_static_table.emplace_back( 18.50, 18.541519, 0.092708 );
    M_static_table.emplace_back( 18.70, 18.727865, 0.093639 );
    M_static_table.emplace_back( 18.90, 18.916083, 0.094580 );
    M_static_table.emplace_back( 19.10, 19.106192, 0.095531 );
    M_static_table.emplace_back( 19.30, 19.298213, 0.096491 );
    M_static_table.emplace_back( 19.50, 19.492163, 0.097461 );
    M_static_table.emplace_back( 19.70, 19.688063, 0.098440 );
    M_static_table.emplace_back( 19.90, 19.885931, 0.099429 );
    M_static_table.emplace_back( 20.10, 20.085788, 0.100429 );
    M_static_table.emplace_back( 20.30, 20.287653, 0.101438 );
    M_static_table.emplace_back( 20.50, 20.491547, 0.102458 );
    M_static_table.emplace_back( 20.70, 20.697491, 0.103487 );
    M_static_table.emplace_back( 20.90, 20.905504, 0.104528 );
    M_static_table.emplace_back( 21.10, 21.115609, 0.105578 );
    M_static_table.emplace_back( 21.30, 21.327824, 0.106639 );
    M_static_table.emplace_back( 21.50, 21.542172, 0.107711 );
    M_static_table.emplace_back( 21.80, 21.758674, 0.108793 );
    M_static_table.emplace_back( 22.00, 21.977353, 0.109887 );
    M_static_table.emplace_back( 22.20, 22.198229, 0.110991 );
    M_static_table.emplace_back( 22.40, 22.421325, 0.112107 );
    M_static_table.emplace_back( 22.60, 22.646663, 0.113233 );
    M_static_table.emplace_back( 22.90, 22.874266, 0.114371 );
    M_static_table.emplace_back( 23.10, 23.104156, 0.115521 );
    M_static_table.emplace_back( 23.30, 23.336357, 0.116682 );
    M_static_table.emplace_back( 23.60, 23.570891, 0.117854 );
    M_static_table.emplace_back( 23.80, 23.807782, 0.119038 );
    M_static_table.emplace_back( 24.00, 24.047054, 0.120235 );
    M_static_table.emplace_back( 24.30, 24.288731, 0.121443 );
    M_static_table.emplace_back( 24.50, 24.532837, 0.122664 );
    M_static_table.emplace_back( 24.80, 24.779396, 0.123896 );
    M_static_table.emplace_back( 25.00, 25.028433, 0.125142 );
    M_static_table.emplace_back( 25.30, 25.279973, 0.126399 );
    M_static_table.emplace_back( 25.50, 25.534041, 0.127670 );
    M_static_table.emplace_back( 25.80, 25.790663, 0.128953 );
    M_static_table.emplace_back( 26.00, 26.049863, 0.130249 );
    M_static_table.emplace_back( 26.30, 26.311668, 0.131558 );
    M_static_table.emplace_back( 26.60, 26.576105, 0.132880 );
    M_static_table.emplace_back( 26.80, 26.843200, 0.134216 );
    M_static_table.emplace_back( 27.10, 27.112978, 0.135564 );
    M_static_table.emplace_back( 27.40, 27.385468, 0.136927 );
    M_static_table.emplace_back( 27.70, 27.660696, 0.138303 );
    M_static_table.emplace_back( 27.90, 27.938691, 0.139693 );
    M_static_table.emplace_back( 28.20, 28.219480, 0.141097 );
    M_static_table.emplace_back( 28.50, 28.503090, 0.142515 );
    M_static_table.emplace_back( 28.80, 28.789551, 0.143947 );
    M_static_table.emplace_back( 29.10, 29.078891, 0.145394 );
    M_static_table.emplace_back( 29.40, 29.371138, 0.146855 );
    M_static_table.emplace_back( 29.70, 29.666323, 0.148331 );
    M_static_table.emplace_back( 30.00, 29.964475, 0.149822 );
    M_static_table.emplace_back( 30.30, 30.265623, 0.151328 );
    M_static_table.emplace_back( 30.60, 30.569797, 0.152848 );
    M_static_table.emplace_back( 30.90, 30.877029, 0.154385 );
    M_static_table.emplace_back( 31.20, 31.187348, 0.155936 );
    M_static_table.emplace_back( 31.50, 31.500786, 0.157503 );
    M_static_table.emplace_back( 31.80, 31.817374, 0.159086 );
    M_static_table.emplace_back( 32.10, 32.137144, 0.160685 );
    M_static_table.emplace_back( 32.50, 32.460128, 0.162300 );
    M_static_table.emplace_back( 32.80, 32.786358, 0.163930 );
    M_static_table.emplace_back( 33.10, 33.115866, 0.165578 );
    M_static_table.emplace_back( 33.40, 33.448686, 0.167242 );
    M_static_table.emplace_back( 33.80, 33.784851, 0.168923 );
    M_static_table.emplace_back( 34.10, 34.124394, 0.170621 );
    M_static_table.emplace_back( 34.50, 34.467350, 0.172336 );
    M_static_table.emplace_back( 34.80, 34.813753, 0.174067 );
    M_static_table.emplace_back( 35.20, 35.163637, 0.175817 );
    M_static_table.emplace_back( 35.50, 35.517037, 0.177584 );
    M_static_table.emplace_back( 35.90, 35.873989, 0.179369 );
    M_static_table.emplace_back( 36.20, 36.234529, 0.181171 );
    M_static_table.emplace_back( 36.60, 36.598691, 0.182992 );
    M_static_table.emplace_back( 37.00, 36.966514, 0.184831 );
    M_static_table.emplace_back( 37.30, 37.338034, 0.186689 );
    M_static_table.emplace_back( 37.70, 37.713288, 0.188565 );
    M_static_table.emplace_back( 38.10, 38.092312, 0.190460 );
    M_static_table.emplace_back( 38.50, 38.475147, 0.192375 );
    M_static_table.emplace_back( 38.90, 38.861829, 0.194308 );
    M_static_table.emplace_back( 39.30, 39.252396, 0.196260 );
    M_static_table.emplace_back( 39.60, 39.646889, 0.198233 );
    M_static_table.emplace_back( 40.00, 40.045347, 0.200225 );
    M_static_table.emplace_back( 40.40, 40.447810, 0.202238 );
    M_static_table.emplace_back( 40.90, 40.854317, 0.204270 );
    M_static_table.emplace_back( 41.30, 41.264910, 0.206323 );
    M_static_table.emplace_back( 41.70, 41.679629, 0.208397 );
    M_static_table.emplace_back( 42.10, 42.098516, 0.210491 );
    M_static_table.emplace_back( 42.50, 42.521613, 0.212606 );
    M_static_table.emplace_back( 42.90, 42.948962, 0.214743 );
    M_static_table.emplace_back( 43.40, 43.380607, 0.216902 );
    M_static_table.emplace_back( 43.80, 43.816589, 0.219081 );
    M_static_table.emplace_back( 44.30, 44.256953, 0.221283 );
    M_static_table.emplace_back( 44.70, 44.701743, 0.223507 );
    M_static_table.emplace_back( 45.20, 45.151003, 0.225753 );
    M_static_table.emplace_back( 45.60, 45.604778, 0.228022 );
    M_static_table.emplace_back( 46.10, 46.063114, 0.230314 );
    M_static_table.emplace_back( 46.50, 46.526056, 0.232629 );
    M_static_table.emplace_back( 47.00, 46.993650, 0.234966 );
    M_static_table.emplace_back( 47.50, 47.465944, 0.237328 );
    M_static_table.emplace_back( 47.90, 47.942985, 0.239713 );
    M_static_table.emplace_back( 48.40, 48.424820, 0.242122 );
    M_static_table.emplace_back( 48.90, 48.911498, 0.244556 );
    M_static_table.emplace_back( 49.40, 49.403066, 0.247013 );
    M_static_table.emplace_back( 49.90, 49.899575, 0.249496 );
    M_static_table.emplace_back( 50.40, 50.401075, 0.252004 );
    M_static_table.emplace_back( 50.90, 50.907614, 0.254536 );
    M_static_table.emplace_back( 51.40, 51.419244, 0.257095 );
    M_static_table.emplace_back( 51.90, 51.936016, 0.259678 );
    M_static_table.emplace_back( 52.50, 52.457981, 0.262288 );
    M_static_table.emplace_back( 53.00, 52.985193, 0.264924 );
    M_static_table.emplace_back( 53.50, 53.517703, 0.267587 );
    M_static_table.emplace_back( 54.10, 54.055565, 0.270276 );
    M_static_table.emplace_back( 54.60, 54.598832, 0.272992 );
    M_static_table.emplace_back( 55.10, 55.147560, 0.275736 );
    M_static_table.emplace_back( 55.70, 55.701802, 0.278507 );
    M_static_table.emplace_back( 56.30, 56.261614, 0.281306 );
    M_static_table.emplace_back( 56.80, 56.827053, 0.284133 );
    M_static_table.emplace_back( 57.40, 57.398175, 0.286989 );
    M_static_table.emplace_back( 58.00, 57.975036, 0.289873 );
    M_static_table.emplace_back( 58.60, 58.557694, 0.292786 );
    M_static_table.emplace_back( 59.10, 59.146209, 0.295729 );
    M_static_table.emplace_back( 59.70, 59.740638, 0.298701 );
    M_static_table.emplace_back( 60.30, 60.341042, 0.301703 );
    M_static_table.emplace_back( 60.90, 60.947479, 0.304735 );
    M_static_table.emplace_back( 61.60, 61.560012, 0.307798 );
    M_static_table.emplace_back( 62.20, 62.178700, 0.310891 );
    M_static_table.emplace_back( 62.80, 62.803606, 0.314015 );
    M_static_table.emplace_back( 63.40, 63.434793, 0.317172 );
    M_static_table.emplace_back( 64.10, 64.072323, 0.320359 );
    M_static_table.emplace_back( 64.70, 64.716261, 0.323579 );
    M_static_table.emplace_back( 65.40, 65.366670, 0.326831 );
    M_static_table.emplace_back( 66.00, 66.023616, 0.330116 );
    M_static_table.emplace_back( 66.70, 66.687164, 0.333433 );
    M_static_table.emplace_back( 67.40, 67.357381, 0.336784 );
    M_static_table.emplace_back( 68.00, 68.034334, 0.340169 );
    M_static_table.emplace_back( 68.70, 68.718091, 0.343588 );
    M_static_table.emplace_back( 69.40, 69.408719, 0.347041 );
    M_static_table.emplace_back( 70.10, 70.106289, 0.350529 );
    M_static_table.emplace_back( 70.80, 70.810869, 0.354052 );
    M_static_table.emplace_back( 71.50, 71.522530, 0.357610 );
    M_static_table.emplace_back( 72.20, 72.241343, 0.361204 );
    M_static_table.emplace_back( 73.00, 72.967380, 0.364834 );
    M_static_table.emplace_back( 73.70, 73.700715, 0.368501 );
    M_static_table.emplace_back( 74.40, 74.441419, 0.372204 );
    M_static_table.emplace_back( 75.20, 75.189568, 0.375945 );
    M_static_table.emplace_back( 75.90, 75.945235, 0.379723 );
    M_static_table.emplace_back( 76.70, 76.708498, 0.383540 );
    M_static_table.emplace_back( 77.50, 77.479431, 0.387394 );
    M_static_table.emplace_back( 78.30, 78.258113, 0.391288 );
    M_static_table.emplace_back( 79.00, 79.044620, 0.395220 );
    M_static_table.emplace_back( 79.80, 79.839031, 0.399192 );
    M_static_table.emplace_back( 80.60, 80.641427, 0.403204 );
    M_static_table.emplace_back( 81.50, 81.451886, 0.407256 );
    M_static_table.emplace_back( 82.30, 82.270492, 0.411350 );
    M_static_table.emplace_back( 83.10, 83.097324, 0.415483 );
    M_static_table.emplace_back( 83.90, 83.932466, 0.419659 );
    M_static_table.emplace_back( 84.80, 84.776001, 0.423876 );
    M_static_table.emplace_back( 85.60, 85.628014, 0.428137 );
    M_static_table.emplace_back( 86.50, 86.488590, 0.432440 );
    M_static_table.emplace_back( 87.40, 87.357815, 0.436786 );
    M_static_table.emplace_back( 88.20, 88.235775, 0.441175 );
    M_static_table.emplace_back( 89.10, 89.122560, 0.445610 );
    M_static_table.emplace_back( 90.00, 90.018257, 0.450088 );
    M_static_table.emplace_back( 90.90, 90.922955, 0.454611 );
    M_static_table.emplace_back( 91.80, 91.836746, 0.459180 );
    M_static_table.emplace_back( 92.80, 92.759720, 0.463795 );
    M_static_table.emplace_back( 93.70, 93.691971, 0.468456 );
    M_static_table.emplace_back( 94.60, 94.633591, 0.473164 );
    M_static_table.emplace_back( 95.60, 95.584675, 0.477920 );
    M_static_table.emplace_back( 96.50, 96.545317, 0.482723 );
    M_static_table.emplace_back( 97.50, 97.515613, 0.487574 );
    M_static_table.emplace_back( 98.50, 98.495661, 0.492474 );
    M_static_table.emplace_back( 99.50, 99.485559, 0.497424 );
    M_static_table.emplace_back( 100.50, 100.485406, 0.502423 );
    M_static_table.emplace_back( 101.50, 101.495301, 0.507473 );
    M_static_table.emplace_back( 102.50, 102.515346, 0.512573 );
    M_static_table.emplace_back( 103.50, 103.545642, 0.517724 );
    M_static_table.emplace_back( 104.60, 104.586293, 0.522928 );
    M_static_table.emplace_back( 105.60, 105.637403, 0.528183 );
    M_static_table.emplace_back( 106.70, 106.699076, 0.533491 );
    M_static_table.emplace_back( 107.80, 107.771420, 0.538853 );
    M_static_table.emplace_back( 108.90, 108.854540, 0.544268 );
    M_static_table.emplace_back( 109.90, 109.948547, 0.549739 );
    M_static_table.emplace_back( 111.10, 111.053548, 0.555263 );
    M_static_table.emplace_back( 112.20, 112.169655, 0.560844 );
    M_static_table.emplace_back( 113.30, 113.296978, 0.566480 );
    M_static_table.emplace_back( 114.40, 114.435632, 0.572174 );
    M_static_table.emplace_back( 115.60, 115.585729, 0.577924 );
    M_static_table.emplace_back( 116.70, 116.747385, 0.583732 );
    M_static_table.emplace_back( 117.90, 117.920716, 0.589599 );
    M_static_table.emplace_back( 119.10, 119.105839, 0.595525 );
    M_static_table.emplace_back( 120.30, 120.302872, 0.601509 );
    M_static_table.emplace_back( 121.50, 121.511936, 0.607555 );
    M_static_table.emplace_back( 122.70, 122.733152, 0.613661 );
    M_static_table.emplace_back( 124.00, 123.966640, 0.619828 );
    M_static_table.emplace_back( 125.20, 125.212526, 0.626058 );
    M_static_table.emplace_back( 126.50, 126.470933, 0.632350 );
    M_static_table.emplace_back( 127.70, 127.741987, 0.638705 );
    M_static_table.emplace_back( 129.00, 129.025815, 0.645124 );
    M_static_table.emplace_back( 130.30, 130.322546, 0.651608 );
    M_static_table.emplace_back( 131.60, 131.632309, 0.658156 );
    M_static_table.emplace_back( 133.00, 132.955236, 0.664771 );
    M_static_table.emplace_back( 134.30, 134.291458, 0.671452 );
    M_static_table.emplace_back( 135.60, 135.641110, 0.678200 );
    M_static_table.emplace_back( 137.00, 137.004326, 0.685016 );
    M_static_table.emplace_back( 138.40, 138.381242, 0.691901 );
    M_static_table.emplace_back( 139.80, 139.771997, 0.698855 );
    M_static_table.emplace_back( 141.20, 141.176729, 0.705878 );
    M_static_table.emplace_back( 142.60, 142.595578, 0.712972 );
    M_static_table.emplace_back( 144.00, 144.028688, 0.720138 );
    M_static_table.emplace_back( 145.50, 145.476200, 0.727375 );
    M_static_table.emplace_back( 146.90, 146.938260, 0.734685 );
    M_static_table.emplace_back( 148.40, 148.415014, 0.742069 );
    M_static_table.emplace_back( 149.90, 149.906610, 0.749527 );
    M_static_table.emplace_back( 151.40, 151.413197, 0.757060 );
    M_static_table.emplace_back( 152.90, 152.934924, 0.764668 );
    M_static_table.emplace_back( 154.50, 154.471946, 0.772354 );
    M_static_table.emplace_back( 156.00, 156.024415, 0.780116 );
    M_static_table.emplace_back( 157.60, 157.592486, 0.787956 );
    M_static_table.emplace_back( 159.20, 159.176317, 0.795875 );
    M_static_table.emplace_back( 160.80, 160.776066, 0.803874 );
    M_static_table.emplace_back( 162.40, 162.391892, 0.811953 );
    M_static_table.emplace_back( 164.00, 164.023958, 0.820113 );
    M_static_table.emplace_back( 165.70, 165.672426, 0.828355 );
    M_static_table.emplace_back( 167.30, 167.337461, 0.836680 );
    M_static_table.emplace_back( 169.00, 169.019231, 0.845089 );
    M_static_table.emplace_back( 170.70, 170.717902, 0.853582 );
    M_static_table.emplace_back( 172.40, 172.433646, 0.862161 );
    M_static_table.emplace_back( 174.20, 174.166633, 0.870826 );
    M_static_table.emplace_back( 175.90, 175.917036, 0.879578 );
    M_static_table.emplace_back( 177.70, 177.685032, 0.888418 );
    M_static_table.emplace_back( 179.50, 179.470796, 0.897347 );

    // --------------------------------------------------------------------
    M_static_table_v18_narrow.emplace_back( 0.00, 0.024956, 0.024956 );
    M_static_table_v18_narrow.emplace_back( 0.10, 0.099928, 0.050016 );
    M_static_table_v18_narrow.emplace_back( 0.20, 0.199822, 0.049878 );
    M_static_table_v18_narrow.emplace_back( 0.30, 0.300256, 0.050557 );
    M_static_table_v18_narrow.emplace_back( 0.40, 0.400634, 0.049820 );
    M_static_table_v18_narrow.emplace_back( 0.50, 0.500320, 0.049866 );
    M_static_table_v18_narrow.emplace_back( 0.60, 0.599535, 0.049350 );
    M_static_table_v18_narrow.emplace_back( 0.70, 0.699511, 0.050626 );
    M_static_table_v18_narrow.emplace_back( 0.80, 0.800077, 0.049940 );
    M_static_table_v18_narrow.emplace_back( 0.90, 0.899436, 0.049419 );
    M_static_table_v18_narrow.emplace_back( 1.00, 0.998751, 0.049896 );
    M_static_table_v18_narrow.emplace_back( 1.10, 1.098024, 0.049377 );
    M_static_table_v18_narrow.emplace_back( 1.20, 1.198299, 0.050897 );
    M_static_table_v18_narrow.emplace_back( 1.30, 1.301217, 0.052021 );
    M_static_table_v18_narrow.emplace_back( 1.40, 1.402298, 0.049060 );
    M_static_table_v18_narrow.emplace_back( 1.50, 1.500095, 0.048736 );
    M_static_table_v18_narrow.emplace_back( 1.60, 1.600840, 0.052009 );
    M_static_table_v18_narrow.emplace_back( 1.70, 1.699575, 0.046726 );
    M_static_table_v18_narrow.emplace_back( 1.80, 1.800294, 0.053992 );
    M_static_table_v18_narrow.emplace_back( 1.90, 1.901822, 0.047535 );
    M_static_table_v18_narrow.emplace_back( 2.00, 1.999331, 0.049973 );
    M_static_table_v18_narrow.emplace_back( 2.10, 2.101839, 0.052535 );
    M_static_table_v18_narrow.emplace_back( 2.20, 2.203954, 0.049581 );
    M_static_table_v18_narrow.emplace_back( 2.30, 2.299519, 0.045985 );
    M_static_table_v18_narrow.emplace_back( 2.40, 2.399483, 0.053979 );
    M_static_table_v18_narrow.emplace_back( 2.50, 2.503526, 0.050064 );
    M_static_table_v18_narrow.emplace_back( 2.60, 2.599069, 0.045479 );
    M_static_table_v18_narrow.emplace_back( 2.70, 2.698511, 0.053963 );
    M_static_table_v18_narrow.emplace_back( 2.80, 2.801495, 0.049021 );
    M_static_table_v18_narrow.emplace_back( 2.90, 2.901284, 0.050768 );
    M_static_table_v18_narrow.emplace_back( 3.00, 3.004627, 0.052576 );
    M_static_table_v18_narrow.emplace_back( 3.10, 3.103755, 0.046553 );
    M_static_table_v18_narrow.emplace_back( 3.20, 3.198279, 0.047971 );
    M_static_table_v18_narrow.emplace_back( 3.30, 3.295681, 0.049432 );
    M_static_table_v18_narrow.emplace_back( 3.40, 3.396049, 0.050937 );
    M_static_table_v18_narrow.emplace_back( 3.50, 3.499474, 0.052488 );
    M_static_table_v18_narrow.emplace_back( 3.60, 3.596921, 0.044959 );
    M_static_table_v18_narrow.emplace_back( 3.70, 3.697336, 0.055456 );
    M_static_table_v18_narrow.emplace_back( 3.80, 3.800294, 0.047502 );
    M_static_table_v18_narrow.emplace_back( 3.90, 3.896499, 0.048704 );
    M_static_table_v18_narrow.emplace_back( 4.00, 3.995139, 0.049937 );
    M_static_table_v18_narrow.emplace_back( 4.10, 4.096276, 0.051201 );
    M_static_table_v18_narrow.emplace_back( 4.20, 4.199974, 0.052497 );
    M_static_table_v18_narrow.emplace_back( 4.30, 4.306297, 0.053826 );
    M_static_table_v18_narrow.emplace_back( 4.40, 4.404163, 0.044041 );
    M_static_table_v18_narrow.emplace_back( 4.50, 4.504506, 0.056303 );
    M_static_table_v18_narrow.emplace_back( 4.60, 4.606877, 0.046068 );
    M_static_table_v18_narrow.emplace_back( 4.70, 4.699942, 0.046998 );
    M_static_table_v18_narrow.emplace_back( 4.80, 4.794887, 0.047947 );
    M_static_table_v18_narrow.emplace_back( 4.90, 4.891750, 0.048916 );
    M_static_table_v18_narrow.emplace_back( 5.00, 4.990570, 0.049904 );
    M_static_table_v18_narrow.emplace_back( 5.10, 5.091386, 0.050912 );
    M_static_table_v18_narrow.emplace_back( 5.20, 5.194239, 0.051941 );
    M_static_table_v18_narrow.emplace_back( 5.30, 5.299169, 0.052990 );
    M_static_table_v18_narrow.emplace_back( 5.40, 5.406220, 0.054061 );
    M_static_table_v18_narrow.emplace_back( 5.50, 5.501541, 0.041261 );
    M_static_table_v18_narrow.emplace_back( 5.60, 5.598788, 0.055986 );
    M_static_table_v18_narrow.emplace_back( 5.70, 5.697505, 0.042731 );
    M_static_table_v18_narrow.emplace_back( 5.80, 5.798215, 0.057980 );
    M_static_table_v18_narrow.emplace_back( 5.90, 5.900448, 0.044253 );
    M_static_table_v18_narrow.emplace_back( 6.00, 6.004746, 0.060046 );
    M_static_table_v18_narrow.emplace_back( 6.10, 6.110620, 0.045829 );
    M_static_table_v18_narrow.emplace_back( 6.20, 6.202970, 0.046521 );
    M_static_table_v18_narrow.emplace_back( 6.30, 6.296716, 0.047225 );
    M_static_table_v18_narrow.emplace_back( 6.40, 6.391879, 0.047939 );
    M_static_table_v18_narrow.emplace_back( 6.50, 6.488480, 0.048663 );
    M_static_table_v18_narrow.emplace_back( 6.60, 6.586540, 0.049398 );
    M_static_table_v18_narrow.emplace_back( 6.70, 6.686083, 0.050145 );
    M_static_table_v18_narrow.emplace_back( 6.80, 6.787131, 0.050903 );
    M_static_table_v18_narrow.emplace_back( 6.90, 6.889705, 0.051672 );
    M_static_table_v18_narrow.emplace_back( 7.00, 6.993829, 0.052453 );
    M_static_table_v18_narrow.emplace_back( 7.10, 7.099528, 0.053246 );
    M_static_table_v18_narrow.emplace_back( 7.20, 7.206823, 0.054050 );
    M_static_table_v18_narrow.emplace_back( 7.30, 7.297360, 0.036487 );
    M_static_table_v18_narrow.emplace_back( 7.40, 7.389265, 0.055419 );
    M_static_table_v18_narrow.emplace_back( 7.50, 7.500939, 0.056256 );
    M_static_table_v18_narrow.emplace_back( 7.60, 7.595171, 0.037976 );
    M_static_table_v18_narrow.emplace_back( 7.70, 7.690826, 0.057680 );
    M_static_table_v18_narrow.emplace_back( 7.80, 7.807058, 0.058552 );
    M_static_table_v18_narrow.emplace_back( 7.90, 7.905135, 0.039525 );
    M_static_table_v18_narrow.emplace_back( 8.00, 8.004695, 0.060034 );
    M_static_table_v18_narrow.emplace_back( 8.10, 8.105255, 0.040525 );
    M_static_table_v18_narrow.emplace_back( 8.20, 8.207334, 0.061554 );
    M_static_table_v18_narrow.emplace_back( 8.30, 8.310440, 0.041552 );
    M_static_table_v18_narrow.emplace_back( 8.40, 8.393962, 0.041969 );
    M_static_table_v18_narrow.emplace_back( 8.50, 8.499678, 0.063746 );
    M_static_table_v18_narrow.emplace_back( 8.60, 8.606456, 0.043031 );
    M_static_table_v18_narrow.emplace_back( 8.70, 8.692952, 0.043464 );
    M_static_table_v18_narrow.emplace_back( 8.80, 8.802433, 0.066017 );
    M_static_table_v18_narrow.emplace_back( 8.90, 8.913015, 0.044565 );
    M_static_table_v18_narrow.emplace_back( 9.00, 9.002593, 0.045012 );
    M_static_table_v18_narrow.emplace_back( 9.10, 9.093070, 0.045465 );
    M_static_table_v18_narrow.emplace_back( 9.20, 9.184456, 0.045921 );
    M_static_table_v18_narrow.emplace_back( 9.30, 9.300128, 0.069750 );
    M_static_table_v18_narrow.emplace_back( 9.40, 9.416962, 0.047084 );
    M_static_table_v18_narrow.emplace_back( 9.50, 9.511604, 0.047557 );
    M_static_table_v18_narrow.emplace_back( 9.60, 9.607197, 0.048035 );
    M_static_table_v18_narrow.emplace_back( 9.70, 9.703751, 0.048518 );
    M_static_table_v18_narrow.emplace_back( 9.80, 9.801276, 0.049006 );
    M_static_table_v18_narrow.emplace_back( 9.90, 9.899780, 0.049498 );
    M_static_table_v18_narrow.emplace_back( 10.00, 9.999275, 0.049996 );
    M_static_table_v18_narrow.emplace_back( 10.10, 10.099769, 0.050498 );
    M_static_table_v18_narrow.emplace_back( 10.20, 10.201273, 0.051006 );
    M_static_table_v18_narrow.emplace_back( 10.30, 10.303797, 0.051518 );
    M_static_table_v18_narrow.emplace_back( 10.40, 10.407352, 0.052036 );
    M_static_table_v18_narrow.emplace_back( 10.50, 10.511948, 0.052559 );
    M_static_table_v18_narrow.emplace_back( 10.60, 10.617595, 0.053087 );
    M_static_table_v18_narrow.emplace_back( 10.70, 10.697426, 0.026743 );
    M_static_table_v18_narrow.emplace_back( 10.80, 10.778059, 0.053889 );
    M_static_table_v18_narrow.emplace_back( 10.90, 10.886380, 0.054431 );
    M_static_table_v18_narrow.emplace_back( 11.00, 10.995790, 0.054978 );
    M_static_table_v18_narrow.emplace_back( 11.10, 11.106300, 0.055531 );
    M_static_table_v18_narrow.emplace_back( 11.20, 11.217920, 0.056089 );
    M_static_table_v18_narrow.emplace_back( 11.30, 11.302265, 0.028255 );
    M_static_table_v18_narrow.emplace_back( 11.40, 11.387457, 0.056936 );
    M_static_table_v18_narrow.emplace_back( 11.50, 11.501903, 0.057509 );
    M_static_table_v18_narrow.emplace_back( 11.60, 11.617500, 0.058087 );
    M_static_table_v18_narrow.emplace_back( 11.70, 11.704849, 0.029262 );
    M_static_table_v18_narrow.emplace_back( 11.80, 11.793075, 0.058964 );
    M_static_table_v18_narrow.emplace_back( 11.90, 11.911597, 0.059557 );
    M_static_table_v18_narrow.emplace_back( 12.00, 12.001158, 0.030003 );
    M_static_table_v18_narrow.emplace_back( 12.10, 12.091618, 0.060457 );
    M_static_table_v18_narrow.emplace_back( 12.20, 12.213141, 0.061065 );
    M_static_table_v18_narrow.emplace_back( 12.30, 12.304969, 0.030762 );
    M_static_table_v18_narrow.emplace_back( 12.40, 12.397719, 0.061988 );
    M_static_table_v18_narrow.emplace_back( 12.50, 12.490935, 0.031227 );
    M_static_table_v18_narrow.emplace_back( 12.60, 12.585087, 0.062925 );
    M_static_table_v18_narrow.emplace_back( 12.70, 12.711569, 0.063557 );
    M_static_table_v18_narrow.emplace_back( 12.80, 12.807144, 0.032017 );
    M_static_table_v18_narrow.emplace_back( 12.90, 12.903680, 0.064518 );
    M_static_table_v18_narrow.emplace_back( 13.00, 13.000699, 0.032501 );
    M_static_table_v18_narrow.emplace_back( 13.10, 13.098694, 0.065493 );
    M_static_table_v18_narrow.emplace_back( 13.20, 13.197180, 0.032993 );
    M_static_table_v18_narrow.emplace_back( 13.30, 13.296655, 0.066482 );
    M_static_table_v18_narrow.emplace_back( 13.40, 13.396629, 0.033491 );
    M_static_table_v18_narrow.emplace_back( 13.50, 13.497608, 0.067487 );
    M_static_table_v18_narrow.emplace_back( 13.60, 13.599094, 0.033998 );
    M_static_table_v18_narrow.emplace_back( 13.70, 13.701599, 0.068507 );
    M_static_table_v18_narrow.emplace_back( 13.80, 13.804617, 0.034511 );
    M_static_table_v18_narrow.emplace_back( 13.90, 13.908672, 0.069543 );
    M_static_table_v18_narrow.emplace_back( 14.00, 14.013248, 0.035033 );
    M_static_table_v18_narrow.emplace_back( 14.10, 14.083489, 0.035208 );
    M_static_table_v18_narrow.emplace_back( 14.20, 14.189646, 0.070948 );
    M_static_table_v18_narrow.emplace_back( 14.30, 14.296334, 0.035740 );
    M_static_table_v18_narrow.emplace_back( 14.40, 14.404095, 0.072020 );
    M_static_table_v18_narrow.emplace_back( 14.50, 14.512396, 0.036281 );
    M_static_table_v18_narrow.emplace_back( 14.60, 14.585139, 0.036462 );
    M_static_table_v18_narrow.emplace_back( 14.70, 14.695077, 0.073475 );
    M_static_table_v18_narrow.emplace_back( 14.80, 14.805565, 0.037013 );
    M_static_table_v18_narrow.emplace_back( 14.90, 14.879778, 0.037199 );
    M_static_table_v18_narrow.emplace_back( 15.00, 14.991937, 0.074959 );
    M_static_table_v18_narrow.emplace_back( 15.10, 15.104657, 0.037761 );
    M_static_table_v18_narrow.emplace_back( 15.20, 15.180370, 0.037951 );
    M_static_table_v18_narrow.emplace_back( 15.30, 15.294794, 0.076473 );
    M_static_table_v18_narrow.emplace_back( 15.40, 15.409792, 0.038524 );
    M_static_table_v18_narrow.emplace_back( 15.50, 15.487034, 0.038718 );
    M_static_table_v18_narrow.emplace_back( 15.60, 15.603770, 0.078018 );
    M_static_table_v18_narrow.emplace_back( 15.70, 15.721090, 0.039302 );
    M_static_table_v18_narrow.emplace_back( 15.80, 15.799892, 0.039499 );
    M_static_table_v18_narrow.emplace_back( 15.90, 15.879090, 0.039698 );
    M_static_table_v18_narrow.emplace_back( 16.00, 15.998781, 0.079993 );
    M_static_table_v18_narrow.emplace_back( 16.10, 16.119071, 0.040298 );
    M_static_table_v18_narrow.emplace_back( 16.20, 16.199869, 0.040500 );
    M_static_table_v18_narrow.emplace_back( 16.30, 16.281071, 0.040703 );
    M_static_table_v18_narrow.emplace_back( 16.40, 16.403792, 0.082018 );
    M_static_table_v18_narrow.emplace_back( 16.50, 16.527128, 0.041318 );
    M_static_table_v18_narrow.emplace_back( 16.60, 16.609970, 0.041525 );
    M_static_table_v18_narrow.emplace_back( 16.70, 16.693228, 0.041733 );
    M_static_table_v18_narrow.emplace_back( 16.80, 16.776903, 0.041943 );
    M_static_table_v18_narrow.emplace_back( 16.90, 16.903362, 0.084516 );
    M_static_table_v18_narrow.emplace_back( 17.00, 17.030454, 0.042576 );
    M_static_table_v18_narrow.emplace_back( 17.10, 17.115819, 0.042790 );
    M_static_table_v18_narrow.emplace_back( 17.20, 17.201613, 0.043004 );
    M_static_table_v18_narrow.emplace_back( 17.30, 17.287836, 0.043220 );
    M_static_table_v18_narrow.emplace_back( 17.40, 17.374492, 0.043436 );
    M_static_table_v18_narrow.emplace_back( 17.50, 17.505454, 0.087527 );
    M_static_table_v18_narrow.emplace_back( 17.60, 17.637073, 0.044093 );
    M_static_table_v18_narrow.emplace_back( 17.70, 17.725480, 0.044314 );
    M_static_table_v18_narrow.emplace_back( 17.80, 17.814329, 0.044536 );
    M_static_table_v18_narrow.emplace_back( 17.90, 17.903624, 0.044759 );
    M_static_table_v18_narrow.emplace_back( 18.00, 17.993366, 0.044984 );
    M_static_table_v18_narrow.emplace_back( 18.10, 18.083558, 0.045209 );
    M_static_table_v18_narrow.emplace_back( 18.20, 18.174202, 0.045436 );
    M_static_table_v18_narrow.emplace_back( 18.30, 18.265301, 0.045663 );
    M_static_table_v18_narrow.emplace_back( 18.40, 18.402978, 0.092015 );
    M_static_table_v18_narrow.emplace_back( 18.50, 18.541346, 0.046353 );
    M_static_table_v18_narrow.emplace_back( 18.60, 18.634284, 0.046586 );
    M_static_table_v18_narrow.emplace_back( 18.70, 18.727689, 0.046820 );
    M_static_table_v18_narrow.emplace_back( 18.80, 18.821562, 0.047054 );
    M_static_table_v18_narrow.emplace_back( 18.90, 18.915906, 0.047290 );
    M_static_table_v18_narrow.emplace_back( 19.00, 19.010722, 0.047527 );
    M_static_table_v18_narrow.emplace_back( 19.10, 19.106014, 0.047765 );
    M_static_table_v18_narrow.emplace_back( 19.20, 19.201783, 0.048005 );
    M_static_table_v18_narrow.emplace_back( 19.30, 19.298033, 0.048245 );
    M_static_table_v18_narrow.emplace_back( 19.40, 19.394764, 0.048487 );
    M_static_table_v18_narrow.emplace_back( 19.50, 19.491981, 0.048730 );
    M_static_table_v18_narrow.emplace_back( 19.60, 19.589685, 0.048974 );
    M_static_table_v18_narrow.emplace_back( 19.70, 19.687878, 0.049220 );
    M_static_table_v18_narrow.emplace_back( 19.80, 19.786564, 0.049467 );
    M_static_table_v18_narrow.emplace_back( 19.90, 19.885745, 0.049714 );
    M_static_table_v18_narrow.emplace_back( 20.00, 19.985422, 0.049964 );
    M_static_table_v18_narrow.emplace_back( 20.10, 20.085600, 0.050214 );
    M_static_table_v18_narrow.emplace_back( 20.20, 20.186279, 0.050466 );
    M_static_table_v18_narrow.emplace_back( 20.30, 20.287463, 0.050719 );
    M_static_table_v18_narrow.emplace_back( 20.40, 20.389155, 0.050973 );
    M_static_table_v18_narrow.emplace_back( 20.50, 20.491356, 0.051229 );
    M_static_table_v18_narrow.emplace_back( 20.60, 20.594070, 0.051485 );
    M_static_table_v18_narrow.emplace_back( 20.70, 20.697298, 0.051743 );
    M_static_table_v18_narrow.emplace_back( 20.80, 20.801043, 0.052003 );
    M_static_table_v18_narrow.emplace_back( 20.90, 20.905309, 0.052263 );
    M_static_table_v18_narrow.emplace_back( 21.00, 21.010097, 0.052525 );
    M_static_table_v18_narrow.emplace_back( 21.10, 21.115411, 0.052789 );
    M_static_table_v18_narrow.emplace_back( 21.20, 21.221252, 0.053053 );
    M_static_table_v18_narrow.emplace_back( 21.30, 21.327624, 0.053319 );
    M_static_table_v18_narrow.emplace_back( 21.40, 21.434530, 0.053587 );
    M_static_table_v18_narrow.emplace_back( 21.50, 21.541971, 0.053855 );
    M_static_table_v18_narrow.emplace_back( 21.60, 21.649950, 0.054125 );
    M_static_table_v18_narrow.emplace_back( 21.80, 21.758471, 0.054396 );
    M_static_table_v18_narrow.emplace_back( 21.90, 21.867536, 0.054669 );
    M_static_table_v18_narrow.emplace_back( 22.00, 21.977147, 0.054943 );
    M_static_table_v18_narrow.emplace_back( 22.10, 22.087308, 0.055218 );
    M_static_table_v18_narrow.emplace_back( 22.20, 22.198021, 0.055495 );
    M_static_table_v18_narrow.emplace_back( 22.30, 22.309289, 0.055773 );
    M_static_table_v18_narrow.emplace_back( 22.40, 22.421115, 0.056053 );
    M_static_table_v18_narrow.emplace_back( 22.50, 22.533502, 0.056334 );
    M_static_table_v18_narrow.emplace_back( 22.60, 22.646451, 0.056616 );
    M_static_table_v18_narrow.emplace_back( 22.80, 22.759967, 0.056900 );
    M_static_table_v18_narrow.emplace_back( 22.90, 22.874052, 0.057185 );
    M_static_table_v18_narrow.emplace_back( 23.00, 22.988709, 0.057472 );
    M_static_table_v18_narrow.emplace_back( 23.10, 23.103940, 0.057760 );
    M_static_table_v18_narrow.emplace_back( 23.20, 23.219749, 0.058050 );
    M_static_table_v18_narrow.emplace_back( 23.30, 23.336138, 0.058340 );
    M_static_table_v18_narrow.emplace_back( 23.50, 23.453111, 0.058633 );
    M_static_table_v18_narrow.emplace_back( 23.60, 23.570671, 0.058927 );
    M_static_table_v18_narrow.emplace_back( 23.70, 23.688819, 0.059222 );
    M_static_table_v18_narrow.emplace_back( 23.80, 23.807559, 0.059519 );
    M_static_table_v18_narrow.emplace_back( 23.90, 23.926895, 0.059817 );
    M_static_table_v18_narrow.emplace_back( 24.00, 24.046829, 0.060117 );
    M_static_table_v18_narrow.emplace_back( 24.20, 24.167365, 0.060419 );
    M_static_table_v18_narrow.emplace_back( 24.30, 24.288504, 0.060721 );
    M_static_table_v18_narrow.emplace_back( 24.40, 24.410251, 0.061026 );
    M_static_table_v18_narrow.emplace_back( 24.50, 24.532608, 0.061332 );
    M_static_table_v18_narrow.emplace_back( 24.70, 24.655578, 0.061639 );
    M_static_table_v18_narrow.emplace_back( 24.80, 24.779164, 0.061948 );
    M_static_table_v18_narrow.emplace_back( 24.90, 24.903371, 0.062259 );
    M_static_table_v18_narrow.emplace_back( 25.00, 25.028199, 0.062570 );
    M_static_table_v18_narrow.emplace_back( 25.20, 25.153653, 0.062884 );
    M_static_table_v18_narrow.emplace_back( 25.30, 25.279737, 0.063200 );
    M_static_table_v18_narrow.emplace_back( 25.40, 25.406452, 0.063516 );
    M_static_table_v18_narrow.emplace_back( 25.50, 25.533802, 0.063834 );
    M_static_table_v18_narrow.emplace_back( 25.70, 25.661791, 0.064155 );
    M_static_table_v18_narrow.emplace_back( 25.80, 25.790421, 0.064476 );
    M_static_table_v18_narrow.emplace_back( 25.90, 25.919696, 0.064799 );
    M_static_table_v18_narrow.emplace_back( 26.00, 26.049619, 0.065124 );
    M_static_table_v18_narrow.emplace_back( 26.20, 26.180194, 0.065451 );
    M_static_table_v18_narrow.emplace_back( 26.30, 26.311422, 0.065778 );
    M_static_table_v18_narrow.emplace_back( 26.40, 26.443309, 0.066109 );
    M_static_table_v18_narrow.emplace_back( 26.60, 26.575857, 0.066440 );
    M_static_table_v18_narrow.emplace_back( 26.70, 26.709069, 0.066773 );
    M_static_table_v18_narrow.emplace_back( 26.80, 26.842948, 0.067107 );
    M_static_table_v18_narrow.emplace_back( 27.00, 26.977499, 0.067444 );
    M_static_table_v18_narrow.emplace_back( 27.10, 27.112724, 0.067782 );
    M_static_table_v18_narrow.emplace_back( 27.20, 27.248628, 0.068122 );
    M_static_table_v18_narrow.emplace_back( 27.40, 27.385212, 0.068463 );
    M_static_table_v18_narrow.emplace_back( 27.50, 27.522481, 0.068807 );
    M_static_table_v18_narrow.emplace_back( 27.70, 27.660438, 0.069151 );
    M_static_table_v18_narrow.emplace_back( 27.80, 27.799086, 0.069498 );
    M_static_table_v18_narrow.emplace_back( 27.90, 27.938430, 0.069846 );
    M_static_table_v18_narrow.emplace_back( 28.10, 28.078471, 0.070196 );
    M_static_table_v18_narrow.emplace_back( 28.20, 28.219215, 0.070548 );
    M_static_table_v18_narrow.emplace_back( 28.40, 28.360665, 0.070902 );
    M_static_table_v18_narrow.emplace_back( 28.50, 28.502823, 0.071257 );
    M_static_table_v18_narrow.emplace_back( 28.60, 28.645694, 0.071614 );
    M_static_table_v18_narrow.emplace_back( 28.80, 28.789281, 0.071973 );
    M_static_table_v18_narrow.emplace_back( 28.90, 28.933588, 0.072334 );
    M_static_table_v18_narrow.emplace_back( 29.10, 29.078619, 0.072697 );
    M_static_table_v18_narrow.emplace_back( 29.20, 29.224376, 0.073061 );
    M_static_table_v18_narrow.emplace_back( 29.40, 29.370863, 0.073427 );
    M_static_table_v18_narrow.emplace_back( 29.50, 29.518086, 0.073796 );
    M_static_table_v18_narrow.emplace_back( 29.70, 29.666046, 0.074165 );
    M_static_table_v18_narrow.emplace_back( 29.80, 29.814747, 0.074537 );
    M_static_table_v18_narrow.emplace_back( 30.00, 29.964195, 0.074911 );
    M_static_table_v18_narrow.emplace_back( 30.10, 30.114391, 0.075286 );
    M_static_table_v18_narrow.emplace_back( 30.30, 30.265340, 0.075664 );
    M_static_table_v18_narrow.emplace_back( 30.40, 30.417045, 0.076042 );
    M_static_table_v18_narrow.emplace_back( 30.60, 30.569511, 0.076424 );
    M_static_table_v18_narrow.emplace_back( 30.70, 30.722742, 0.076807 );
    M_static_table_v18_narrow.emplace_back( 30.90, 30.876740, 0.077192 );
    M_static_table_v18_narrow.emplace_back( 31.00, 31.031510, 0.077579 );
    M_static_table_v18_narrow.emplace_back( 31.20, 31.187057, 0.077968 );
    M_static_table_v18_narrow.emplace_back( 31.30, 31.343382, 0.078358 );
    M_static_table_v18_narrow.emplace_back( 31.50, 31.500491, 0.078751 );
    M_static_table_v18_narrow.emplace_back( 31.70, 31.658388, 0.079146 );
    M_static_table_v18_narrow.emplace_back( 31.80, 31.817077, 0.079543 );
    M_static_table_v18_narrow.emplace_back( 32.00, 31.976560, 0.079941 );
    M_static_table_v18_narrow.emplace_back( 32.10, 32.136843, 0.080342 );
    M_static_table_v18_narrow.emplace_back( 32.30, 32.297930, 0.080745 );
    M_static_table_v18_narrow.emplace_back( 32.50, 32.459824, 0.081149 );
    M_static_table_v18_narrow.emplace_back( 32.60, 32.622530, 0.081556 );
    M_static_table_v18_narrow.emplace_back( 32.80, 32.786051, 0.081965 );
    M_static_table_v18_narrow.emplace_back( 33.00, 32.950392, 0.082375 );
    M_static_table_v18_narrow.emplace_back( 33.10, 33.115556, 0.082789 );
    M_static_table_v18_narrow.emplace_back( 33.30, 33.281549, 0.083203 );
    M_static_table_v18_narrow.emplace_back( 33.40, 33.448373, 0.083621 );
    M_static_table_v18_narrow.emplace_back( 33.60, 33.616034, 0.084039 );
    M_static_table_v18_narrow.emplace_back( 33.80, 33.784535, 0.084461 );
    M_static_table_v18_narrow.emplace_back( 34.00, 33.953881, 0.084884 );
    M_static_table_v18_narrow.emplace_back( 34.10, 34.124075, 0.085310 );
    M_static_table_v18_narrow.emplace_back( 34.30, 34.295123, 0.085737 );
    M_static_table_v18_narrow.emplace_back( 34.50, 34.467028, 0.086167 );
    M_static_table_v18_narrow.emplace_back( 34.60, 34.639794, 0.086599 );
    M_static_table_v18_narrow.emplace_back( 34.80, 34.813427, 0.087033 );
    M_static_table_v18_narrow.emplace_back( 35.00, 34.987930, 0.087469 );
    M_static_table_v18_narrow.emplace_back( 35.20, 35.163308, 0.087908 );
    M_static_table_v18_narrow.emplace_back( 35.30, 35.339565, 0.088348 );
    M_static_table_v18_narrow.emplace_back( 35.50, 35.516705, 0.088791 );
    M_static_table_v18_narrow.emplace_back( 35.70, 35.694733, 0.089237 );
    M_static_table_v18_narrow.emplace_back( 35.90, 35.873654, 0.089683 );
    M_static_table_v18_narrow.emplace_back( 36.10, 36.053471, 0.090133 );
    M_static_table_v18_narrow.emplace_back( 36.20, 36.234190, 0.090585 );
    M_static_table_v18_narrow.emplace_back( 36.40, 36.415815, 0.091039 );
    M_static_table_v18_narrow.emplace_back( 36.60, 36.598349, 0.091495 );
    M_static_table_v18_narrow.emplace_back( 36.80, 36.781799, 0.091954 );
    M_static_table_v18_narrow.emplace_back( 37.00, 36.966169, 0.092415 );
    M_static_table_v18_narrow.emplace_back( 37.20, 37.151462, 0.092878 );
    M_static_table_v18_narrow.emplace_back( 37.30, 37.337685, 0.093344 );
    M_static_table_v18_narrow.emplace_back( 37.50, 37.524841, 0.093812 );
    M_static_table_v18_narrow.emplace_back( 37.70, 37.712935, 0.094282 );
    M_static_table_v18_narrow.emplace_back( 37.90, 37.901972, 0.094755 );
    M_static_table_v18_narrow.emplace_back( 38.10, 38.091956, 0.095229 );
    M_static_table_v18_narrow.emplace_back( 38.30, 38.282893, 0.095707 );
    M_static_table_v18_narrow.emplace_back( 38.50, 38.474787, 0.096187 );
    M_static_table_v18_narrow.emplace_back( 38.70, 38.667642, 0.096668 );
    M_static_table_v18_narrow.emplace_back( 38.90, 38.861464, 0.097153 );
    M_static_table_v18_narrow.emplace_back( 39.10, 39.056258, 0.097640 );
    M_static_table_v18_narrow.emplace_back( 39.30, 39.252029, 0.098130 );
    M_static_table_v18_narrow.emplace_back( 39.40, 39.448780, 0.098621 );
    M_static_table_v18_narrow.emplace_back( 39.60, 39.646518, 0.099116 );
    M_static_table_v18_narrow.emplace_back( 39.80, 39.845247, 0.099612 );
    M_static_table_v18_narrow.emplace_back( 40.00, 40.044972, 0.100112 );
    M_static_table_v18_narrow.emplace_back( 40.20, 40.245699, 0.100614 );
    M_static_table_v18_narrow.emplace_back( 40.40, 40.447431, 0.101118 );
    M_static_table_v18_narrow.emplace_back( 40.70, 40.650175, 0.101625 );
    M_static_table_v18_narrow.emplace_back( 40.90, 40.853934, 0.102134 );
    M_static_table_v18_narrow.emplace_back( 41.10, 41.058715, 0.102646 );
    M_static_table_v18_narrow.emplace_back( 41.30, 41.264523, 0.103161 );
    M_static_table_v18_narrow.emplace_back( 41.50, 41.471363, 0.103678 );
    M_static_table_v18_narrow.emplace_back( 41.70, 41.679239, 0.104198 );
    M_static_table_v18_narrow.emplace_back( 41.90, 41.888157, 0.104720 );
    M_static_table_v18_narrow.emplace_back( 42.10, 42.098122, 0.105245 );
    M_static_table_v18_narrow.emplace_back( 42.30, 42.309140, 0.105773 );
    M_static_table_v18_narrow.emplace_back( 42.50, 42.521215, 0.106302 );
    M_static_table_v18_narrow.emplace_back( 42.70, 42.734354, 0.106836 );
    M_static_table_v18_narrow.emplace_back( 42.90, 42.948561, 0.107371 );
    M_static_table_v18_narrow.emplace_back( 43.20, 43.163841, 0.107909 );
    M_static_table_v18_narrow.emplace_back( 43.40, 43.380201, 0.108450 );
    M_static_table_v18_narrow.emplace_back( 43.60, 43.597645, 0.108994 );
    M_static_table_v18_narrow.emplace_back( 43.80, 43.816179, 0.109540 );
    M_static_table_v18_narrow.emplace_back( 44.00, 44.035808, 0.110089 );
    M_static_table_v18_narrow.emplace_back( 44.30, 44.256539, 0.110641 );
    M_static_table_v18_narrow.emplace_back( 44.50, 44.478376, 0.111196 );
    M_static_table_v18_narrow.emplace_back( 44.70, 44.701325, 0.111753 );
    M_static_table_v18_narrow.emplace_back( 44.90, 44.925391, 0.112313 );
    M_static_table_v18_narrow.emplace_back( 45.20, 45.150580, 0.112876 );
    M_static_table_v18_narrow.emplace_back( 45.40, 45.376899, 0.113442 );
    M_static_table_v18_narrow.emplace_back( 45.60, 45.604351, 0.114010 );
    M_static_table_v18_narrow.emplace_back( 45.80, 45.832944, 0.114582 );
    M_static_table_v18_narrow.emplace_back( 46.10, 46.062682, 0.115156 );
    M_static_table_v18_narrow.emplace_back( 46.30, 46.293573, 0.115734 );
    M_static_table_v18_narrow.emplace_back( 46.50, 46.525620, 0.116313 );
    M_static_table_v18_narrow.emplace_back( 46.80, 46.758831, 0.116897 );
    M_static_table_v18_narrow.emplace_back( 47.00, 46.993210, 0.117482 );
    M_static_table_v18_narrow.emplace_back( 47.20, 47.228765, 0.118072 );
    M_static_table_v18_narrow.emplace_back( 47.50, 47.465500, 0.118663 );
    M_static_table_v18_narrow.emplace_back( 47.70, 47.703422, 0.119258 );
    M_static_table_v18_narrow.emplace_back( 47.90, 47.942536, 0.119856 );
    M_static_table_v18_narrow.emplace_back( 48.20, 48.182849, 0.120456 );
    M_static_table_v18_narrow.emplace_back( 48.40, 48.424367, 0.121061 );
    M_static_table_v18_narrow.emplace_back( 48.70, 48.667095, 0.121667 );
    M_static_table_v18_narrow.emplace_back( 48.90, 48.911040, 0.122277 );
    M_static_table_v18_narrow.emplace_back( 49.20, 49.156207, 0.122890 );
    M_static_table_v18_narrow.emplace_back( 49.40, 49.402604, 0.123506 );
    M_static_table_v18_narrow.emplace_back( 49.70, 49.650235, 0.124125 );
    M_static_table_v18_narrow.emplace_back( 49.90, 49.899108, 0.124747 );
    M_static_table_v18_narrow.emplace_back( 50.10, 50.149229, 0.125373 );
    M_static_table_v18_narrow.emplace_back( 50.40, 50.400603, 0.126001 );
    M_static_table_v18_narrow.emplace_back( 50.70, 50.653237, 0.126633 );
    M_static_table_v18_narrow.emplace_back( 50.90, 50.907137, 0.127267 );
    M_static_table_v18_narrow.emplace_back( 51.20, 51.162310, 0.127905 );
    M_static_table_v18_narrow.emplace_back( 51.40, 51.418762, 0.128546 );
    M_static_table_v18_narrow.emplace_back( 51.70, 51.676500, 0.129191 );
    M_static_table_v18_narrow.emplace_back( 51.90, 51.935529, 0.129838 );
    M_static_table_v18_narrow.emplace_back( 52.20, 52.195857, 0.130489 );
    M_static_table_v18_narrow.emplace_back( 52.50, 52.457490, 0.131143 );
    M_static_table_v18_narrow.emplace_back( 52.70, 52.720434, 0.131800 );
    M_static_table_v18_narrow.emplace_back( 53.00, 52.984696, 0.132461 );
    M_static_table_v18_narrow.emplace_back( 53.30, 53.250283, 0.133125 );
    M_static_table_v18_narrow.emplace_back( 53.50, 53.517202, 0.133793 );
    M_static_table_v18_narrow.emplace_back( 53.80, 53.785458, 0.134463 );
    M_static_table_v18_narrow.emplace_back( 54.10, 54.055058, 0.135137 );
    M_static_table_v18_narrow.emplace_back( 54.30, 54.326011, 0.135815 );
    M_static_table_v18_narrow.emplace_back( 54.60, 54.598321, 0.136495 );
    M_static_table_v18_narrow.emplace_back( 54.90, 54.871996, 0.137179 );
    M_static_table_v18_narrow.emplace_back( 55.10, 55.147043, 0.137867 );
    M_static_table_v18_narrow.emplace_back( 55.40, 55.423469, 0.138558 );
    M_static_table_v18_narrow.emplace_back( 55.70, 55.701280, 0.139252 );
    M_static_table_v18_narrow.emplace_back( 56.00, 55.980484, 0.139951 );
    M_static_table_v18_narrow.emplace_back( 56.30, 56.261087, 0.140652 );
    M_static_table_v18_narrow.emplace_back( 56.50, 56.543097, 0.141357 );
    M_static_table_v18_narrow.emplace_back( 56.80, 56.826521, 0.142066 );
    M_static_table_v18_narrow.emplace_back( 57.10, 57.111365, 0.142778 );
    M_static_table_v18_narrow.emplace_back( 57.40, 57.397637, 0.143494 );
    M_static_table_v18_narrow.emplace_back( 57.70, 57.685344, 0.144213 );
    M_static_table_v18_narrow.emplace_back( 58.00, 57.974493, 0.144936 );
    M_static_table_v18_narrow.emplace_back( 58.30, 58.265091, 0.145662 );
    M_static_table_v18_narrow.emplace_back( 58.60, 58.557146, 0.146392 );
    M_static_table_v18_narrow.emplace_back( 58.90, 58.850665, 0.147126 );
    M_static_table_v18_narrow.emplace_back( 59.10, 59.145655, 0.147864 );
    M_static_table_v18_narrow.emplace_back( 59.40, 59.442124, 0.148605 );
    M_static_table_v18_narrow.emplace_back( 59.70, 59.740079, 0.149350 );
    M_static_table_v18_narrow.emplace_back( 60.00, 60.039527, 0.150098 );
    M_static_table_v18_narrow.emplace_back( 60.30, 60.340477, 0.150851 );
    M_static_table_v18_narrow.emplace_back( 60.60, 60.642935, 0.151607 );
    M_static_table_v18_narrow.emplace_back( 60.90, 60.946909, 0.152367 );
    M_static_table_v18_narrow.emplace_back( 61.30, 61.252406, 0.153130 );
    M_static_table_v18_narrow.emplace_back( 61.60, 61.559435, 0.153898 );
    M_static_table_v18_narrow.emplace_back( 61.90, 61.868003, 0.154670 );
    M_static_table_v18_narrow.emplace_back( 62.20, 62.178118, 0.155445 );
    M_static_table_v18_narrow.emplace_back( 62.50, 62.489787, 0.156224 );
    M_static_table_v18_narrow.emplace_back( 62.80, 62.803018, 0.157007 );
    M_static_table_v18_narrow.emplace_back( 63.10, 63.117819, 0.157794 );
    M_static_table_v18_narrow.emplace_back( 63.40, 63.434199, 0.158585 );
    M_static_table_v18_narrow.emplace_back( 63.80, 63.752164, 0.159380 );
    M_static_table_v18_narrow.emplace_back( 64.10, 64.071723, 0.160179 );
    M_static_table_v18_narrow.emplace_back( 64.40, 64.392884, 0.160982 );
    M_static_table_v18_narrow.emplace_back( 64.70, 64.715655, 0.161789 );
    M_static_table_v18_narrow.emplace_back( 65.00, 65.040043, 0.162599 );
    M_static_table_v18_narrow.emplace_back( 65.40, 65.366058, 0.163415 );
    M_static_table_v18_narrow.emplace_back( 65.70, 65.693707, 0.164234 );
    M_static_table_v18_narrow.emplace_back( 66.00, 66.022998, 0.165057 );
    M_static_table_v18_narrow.emplace_back( 66.40, 66.353939, 0.165884 );
    M_static_table_v18_narrow.emplace_back( 66.70, 66.686540, 0.166716 );
    M_static_table_v18_narrow.emplace_back( 67.00, 67.020807, 0.167551 );
    M_static_table_v18_narrow.emplace_back( 67.40, 67.356750, 0.168391 );
    M_static_table_v18_narrow.emplace_back( 67.70, 67.694378, 0.169236 );
    M_static_table_v18_narrow.emplace_back( 68.00, 68.033697, 0.170083 );
    M_static_table_v18_narrow.emplace_back( 68.40, 68.374717, 0.170936 );
    M_static_table_v18_narrow.emplace_back( 68.70, 68.717447, 0.171793 );
    M_static_table_v18_narrow.emplace_back( 69.10, 69.061895, 0.172654 );
    M_static_table_v18_narrow.emplace_back( 69.40, 69.408069, 0.173519 );
    M_static_table_v18_narrow.emplace_back( 69.80, 69.755978, 0.174389 );
    M_static_table_v18_narrow.emplace_back( 70.10, 70.105632, 0.175264 );
    M_static_table_v18_narrow.emplace_back( 70.50, 70.457038, 0.176142 );
    M_static_table_v18_narrow.emplace_back( 70.80, 70.810205, 0.177025 );
    M_static_table_v18_narrow.emplace_back( 71.20, 71.165142, 0.177912 );
    M_static_table_v18_narrow.emplace_back( 71.50, 71.521859, 0.178804 );
    M_static_table_v18_narrow.emplace_back( 71.90, 71.880364, 0.179700 );
    M_static_table_v18_narrow.emplace_back( 72.20, 72.240666, 0.180601 );
    M_static_table_v18_narrow.emplace_back( 72.60, 72.602774, 0.181506 );
    M_static_table_v18_narrow.emplace_back( 73.00, 72.966697, 0.182416 );
    M_static_table_v18_narrow.emplace_back( 73.30, 73.332444, 0.183331 );
    M_static_table_v18_narrow.emplace_back( 73.70, 73.700024, 0.184249 );
    M_static_table_v18_narrow.emplace_back( 74.10, 74.069447, 0.185173 );
    M_static_table_v18_narrow.emplace_back( 74.40, 74.440722, 0.186101 );
    M_static_table_v18_narrow.emplace_back( 74.80, 74.813858, 0.187034 );
    M_static_table_v18_narrow.emplace_back( 75.20, 75.188864, 0.187972 );
    M_static_table_v18_narrow.emplace_back( 75.60, 75.565750, 0.188914 );
    M_static_table_v18_narrow.emplace_back( 75.90, 75.944524, 0.189860 );
    M_static_table_v18_narrow.emplace_back( 76.30, 76.325198, 0.190813 );
    M_static_table_v18_narrow.emplace_back( 76.70, 76.707780, 0.191769 );
    M_static_table_v18_narrow.emplace_back( 77.10, 77.092279, 0.192730 );
    M_static_table_v18_narrow.emplace_back( 77.50, 77.478705, 0.193696 );
    M_static_table_v18_narrow.emplace_back( 77.90, 77.867069, 0.194667 );
    M_static_table_v18_narrow.emplace_back( 78.30, 78.257380, 0.195643 );
    M_static_table_v18_narrow.emplace_back( 78.60, 78.649646, 0.196623 );
    M_static_table_v18_narrow.emplace_back( 79.00, 79.043879, 0.197609 );
    M_static_table_v18_narrow.emplace_back( 79.40, 79.440088, 0.198599 );
    M_static_table_v18_narrow.emplace_back( 79.80, 79.838283, 0.199595 );
    M_static_table_v18_narrow.emplace_back( 80.20, 80.238474, 0.200595 );
    M_static_table_v18_narrow.emplace_back( 80.60, 80.640671, 0.201601 );
    M_static_table_v18_narrow.emplace_back( 81.00, 81.044884, 0.202611 );
    M_static_table_v18_narrow.emplace_back( 81.50, 81.451123, 0.203627 );
    M_static_table_v18_narrow.emplace_back( 81.90, 81.859399, 0.204648 );
    M_static_table_v18_narrow.emplace_back( 82.30, 82.269721, 0.205674 );
    M_static_table_v18_narrow.emplace_back( 82.70, 82.682100, 0.206705 );
    M_static_table_v18_narrow.emplace_back( 83.10, 83.096546, 0.207741 );
    M_static_table_v18_narrow.emplace_back( 83.50, 83.513069, 0.208782 );
    M_static_table_v18_narrow.emplace_back( 83.90, 83.931680, 0.209829 );
    M_static_table_v18_narrow.emplace_back( 84.40, 84.352389, 0.210880 );
    M_static_table_v18_narrow.emplace_back( 84.80, 84.775207, 0.211937 );
    M_static_table_v18_narrow.emplace_back( 85.20, 85.200145, 0.213000 );
    M_static_table_v18_narrow.emplace_back( 85.60, 85.627212, 0.214067 );
    M_static_table_v18_narrow.emplace_back( 86.10, 86.056420, 0.215140 );
    M_static_table_v18_narrow.emplace_back( 86.50, 86.487780, 0.216219 );
    M_static_table_v18_narrow.emplace_back( 86.90, 86.921302, 0.217303 );
    M_static_table_v18_narrow.emplace_back( 87.40, 87.356997, 0.218392 );
    M_static_table_v18_narrow.emplace_back( 87.80, 87.794875, 0.219486 );
    M_static_table_v18_narrow.emplace_back( 88.20, 88.234949, 0.220587 );
    M_static_table_v18_narrow.emplace_back( 88.70, 88.677228, 0.221692 );
    M_static_table_v18_narrow.emplace_back( 89.10, 89.121725, 0.222804 );
    M_static_table_v18_narrow.emplace_back( 89.60, 89.568449, 0.223920 );
    M_static_table_v18_narrow.emplace_back( 90.00, 90.017413, 0.225043 );
    M_static_table_v18_narrow.emplace_back( 90.50, 90.468627, 0.226171 );
    M_static_table_v18_narrow.emplace_back( 90.90, 90.922103, 0.227305 );
    M_static_table_v18_narrow.emplace_back( 91.40, 91.377852, 0.228444 );
    M_static_table_v18_narrow.emplace_back( 91.80, 91.835885, 0.229589 );
    M_static_table_v18_narrow.emplace_back( 92.30, 92.296215, 0.230740 );
    M_static_table_v18_narrow.emplace_back( 92.80, 92.758851, 0.231896 );
    M_static_table_v18_narrow.emplace_back( 93.20, 93.223807, 0.233059 );
    M_static_table_v18_narrow.emplace_back( 93.70, 93.691093, 0.234227 );
    M_static_table_v18_narrow.emplace_back( 94.20, 94.160722, 0.235401 );
    M_static_table_v18_narrow.emplace_back( 94.60, 94.632704, 0.236581 );
    M_static_table_v18_narrow.emplace_back( 95.10, 95.107053, 0.237767 );
    M_static_table_v18_narrow.emplace_back( 95.60, 95.583779, 0.238959 );
    M_static_table_v18_narrow.emplace_back( 96.10, 96.062895, 0.240157 );
    M_static_table_v18_narrow.emplace_back( 96.50, 96.544412, 0.241360 );
    M_static_table_v18_narrow.emplace_back( 97.00, 97.028343, 0.242570 );
    M_static_table_v18_narrow.emplace_back( 97.50, 97.514699, 0.243786 );
    M_static_table_v18_narrow.emplace_back( 98.00, 98.003494, 0.245008 );
    M_static_table_v18_narrow.emplace_back( 98.50, 98.494738, 0.246236 );
    M_static_table_v18_narrow.emplace_back( 99.00, 98.988445, 0.247470 );
    M_static_table_v18_narrow.emplace_back( 99.50, 99.484627, 0.248711 );
    M_static_table_v18_narrow.emplace_back( 100.00, 99.983296, 0.249958 );
    M_static_table_v18_narrow.emplace_back( 100.50, 100.484464, 0.251210 );
    M_static_table_v18_narrow.emplace_back( 101.00, 100.988145, 0.252470 );
    M_static_table_v18_narrow.emplace_back( 101.50, 101.494350, 0.253735 );
    M_static_table_v18_narrow.emplace_back( 102.00, 102.003092, 0.255007 );
    M_static_table_v18_narrow.emplace_back( 102.50, 102.514385, 0.256285 );
    M_static_table_v18_narrow.emplace_back( 103.00, 103.028241, 0.257570 );
    M_static_table_v18_narrow.emplace_back( 103.50, 103.544672, 0.258861 );
    M_static_table_v18_narrow.emplace_back( 104.10, 104.063691, 0.260158 );
    M_static_table_v18_narrow.emplace_back( 104.60, 104.585313, 0.261463 );
    M_static_table_v18_narrow.emplace_back( 105.10, 105.109549, 0.262773 );
    M_static_table_v18_narrow.emplace_back( 105.60, 105.636412, 0.264090 );
    M_static_table_v18_narrow.emplace_back( 106.20, 106.165917, 0.265414 );
    M_static_table_v18_narrow.emplace_back( 106.70, 106.698076, 0.266744 );
    M_static_table_v18_narrow.emplace_back( 107.20, 107.232903, 0.268082 );
    M_static_table_v18_narrow.emplace_back( 107.80, 107.770410, 0.269425 );
    M_static_table_v18_narrow.emplace_back( 108.30, 108.310611, 0.270776 );
    M_static_table_v18_narrow.emplace_back( 108.90, 108.853520, 0.272133 );
    M_static_table_v18_narrow.emplace_back( 109.40, 109.399151, 0.273497 );
    M_static_table_v18_narrow.emplace_back( 109.90, 109.947517, 0.274868 );
    M_static_table_v18_narrow.emplace_back( 110.50, 110.498631, 0.276246 );
    M_static_table_v18_narrow.emplace_back( 111.10, 111.052507, 0.277630 );
    M_static_table_v18_narrow.emplace_back( 111.60, 111.609160, 0.279022 );
    M_static_table_v18_narrow.emplace_back( 112.20, 112.168604, 0.280421 );
    M_static_table_v18_narrow.emplace_back( 112.70, 112.730851, 0.281826 );
    M_static_table_v18_narrow.emplace_back( 113.30, 113.295917, 0.283239 );
    M_static_table_v18_narrow.emplace_back( 113.90, 113.863815, 0.284659 );
    M_static_table_v18_narrow.emplace_back( 114.40, 114.434560, 0.286086 );
    M_static_table_v18_narrow.emplace_back( 115.00, 115.008165, 0.287519 );
    M_static_table_v18_narrow.emplace_back( 115.60, 115.584646, 0.288961 );
    M_static_table_v18_narrow.emplace_back( 116.20, 116.164016, 0.290409 );
    M_static_table_v18_narrow.emplace_back( 116.70, 116.746291, 0.291865 );
    M_static_table_v18_narrow.emplace_back( 117.30, 117.331485, 0.293328 );
    M_static_table_v18_narrow.emplace_back( 117.90, 117.919611, 0.294798 );
    M_static_table_v18_narrow.emplace_back( 118.50, 118.510686, 0.296276 );
    M_static_table_v18_narrow.emplace_back( 119.10, 119.104723, 0.297761 );
    M_static_table_v18_narrow.emplace_back( 119.70, 119.701738, 0.299254 );
    M_static_table_v18_narrow.emplace_back( 120.30, 120.301745, 0.300753 );
    M_static_table_v18_narrow.emplace_back( 120.90, 120.904760, 0.302261 );
    M_static_table_v18_narrow.emplace_back( 121.50, 121.510798, 0.303776 );
    M_static_table_v18_narrow.emplace_back( 122.10, 122.119873, 0.305299 );
    M_static_table_v18_narrow.emplace_back( 122.70, 122.732001, 0.306829 );
    M_static_table_v18_narrow.emplace_back( 123.30, 123.347198, 0.308367 );
    M_static_table_v18_narrow.emplace_back( 124.00, 123.965479, 0.309913 );
    M_static_table_v18_narrow.emplace_back( 124.60, 124.586858, 0.311466 );
    M_static_table_v18_narrow.emplace_back( 125.20, 125.211352, 0.313027 );
    M_static_table_v18_narrow.emplace_back( 125.80, 125.838977, 0.314597 );
    M_static_table_v18_narrow.emplace_back( 126.50, 126.469747, 0.316173 );
    M_static_table_v18_narrow.emplace_back( 127.10, 127.103679, 0.317758 );
    M_static_table_v18_narrow.emplace_back( 127.70, 127.740789, 0.319351 );
    M_static_table_v18_narrow.emplace_back( 128.40, 128.381093, 0.320952 );
    M_static_table_v18_narrow.emplace_back( 129.00, 129.024606, 0.322561 );
    M_static_table_v18_narrow.emplace_back( 129.70, 129.671344, 0.324177 );
    M_static_table_v18_narrow.emplace_back( 130.30, 130.321325, 0.325803 );
    M_static_table_v18_narrow.emplace_back( 131.00, 130.974563, 0.327435 );
    M_static_table_v18_narrow.emplace_back( 131.60, 131.631076, 0.329077 );
    M_static_table_v18_narrow.emplace_back( 132.30, 132.290879, 0.330726 );
    M_static_table_v18_narrow.emplace_back( 133.00, 132.953990, 0.332384 );
    M_static_table_v18_narrow.emplace_back( 133.60, 133.620425, 0.334050 );
    M_static_table_v18_narrow.emplace_back( 134.30, 134.290200, 0.335725 );
    M_static_table_v18_narrow.emplace_back( 135.00, 134.963332, 0.337407 );
    M_static_table_v18_narrow.emplace_back( 135.60, 135.639839, 0.339099 );
    M_static_table_v18_narrow.emplace_back( 136.30, 136.319736, 0.340798 );
    M_static_table_v18_narrow.emplace_back( 137.00, 137.003042, 0.342507 );
    M_static_table_v18_narrow.emplace_back( 137.70, 137.689772, 0.344223 );
    M_static_table_v18_narrow.emplace_back( 138.40, 138.379945, 0.345949 );
    M_static_table_v18_narrow.emplace_back( 139.10, 139.073578, 0.347683 );
    M_static_table_v18_narrow.emplace_back( 139.80, 139.770687, 0.349426 );
    M_static_table_v18_narrow.emplace_back( 140.50, 140.471290, 0.351177 );
    M_static_table_v18_narrow.emplace_back( 141.20, 141.175406, 0.352938 );
    M_static_table_v18_narrow.emplace_back( 141.90, 141.883050, 0.354706 );
    M_static_table_v18_narrow.emplace_back( 142.60, 142.594242, 0.356485 );
    M_static_table_v18_narrow.emplace_back( 143.30, 143.308999, 0.358272 );
    M_static_table_v18_narrow.emplace_back( 144.00, 144.027338, 0.360067 );
    M_static_table_v18_narrow.emplace_back( 144.70, 144.749278, 0.361872 );
    M_static_table_v18_narrow.emplace_back( 145.50, 145.474837, 0.363686 );
    M_static_table_v18_narrow.emplace_back( 146.20, 146.204032, 0.365509 );
    M_static_table_v18_narrow.emplace_back( 146.90, 146.936883, 0.367341 );
    M_static_table_v18_narrow.emplace_back( 147.70, 147.673407, 0.369182 );
    M_static_table_v18_narrow.emplace_back( 148.40, 148.413623, 0.371033 );
    M_static_table_v18_narrow.emplace_back( 149.20, 149.157550, 0.372893 );
    M_static_table_v18_narrow.emplace_back( 149.90, 149.905205, 0.374762 );
    M_static_table_v18_narrow.emplace_back( 150.70, 150.656608, 0.376640 );
    M_static_table_v18_narrow.emplace_back( 151.40, 151.411777, 0.378528 );
    M_static_table_v18_narrow.emplace_back( 152.20, 152.170732, 0.380426 );

    // --------------------------------------------------------------------
    M_static_table_v18_normal.emplace_back( 0.00, 0.024988, 0.024988 );
    M_static_table_v18_normal.emplace_back( 0.10, 0.100241, 0.050266 );
    M_static_table_v18_normal.emplace_back( 0.20, 0.200572, 0.050065 );
    M_static_table_v18_normal.emplace_back( 0.30, 0.300945, 0.050308 );
    M_static_table_v18_normal.emplace_back( 0.40, 0.400572, 0.049319 );
    M_static_table_v18_normal.emplace_back( 0.50, 0.500382, 0.050492 );
    M_static_table_v18_normal.emplace_back( 0.60, 0.600285, 0.049412 );
    M_static_table_v18_normal.emplace_back( 0.70, 0.699448, 0.049752 );
    M_static_table_v18_normal.emplace_back( 0.80, 0.800140, 0.050940 );
    M_static_table_v18_normal.emplace_back( 0.90, 0.901750, 0.050670 );
    M_static_table_v18_normal.emplace_back( 1.00, 1.001189, 0.048769 );
    M_static_table_v18_normal.emplace_back( 1.10, 1.099397, 0.049439 );
    M_static_table_v18_normal.emplace_back( 1.20, 1.198236, 0.049399 );
    M_static_table_v18_normal.emplace_back( 1.30, 1.301283, 0.053647 );
    M_static_table_v18_normal.emplace_back( 1.40, 1.402238, 0.047307 );
    M_static_table_v18_normal.emplace_back( 1.50, 1.500157, 0.050611 );
    M_static_table_v18_normal.emplace_back( 1.60, 1.598715, 0.047947 );
    M_static_table_v18_normal.emplace_back( 1.70, 1.697574, 0.050912 );
    M_static_table_v18_normal.emplace_back( 1.80, 1.802546, 0.054060 );
    M_static_table_v18_normal.emplace_back( 1.90, 1.906644, 0.050038 );
    M_static_table_v18_normal.emplace_back( 2.00, 2.001713, 0.045031 );
    M_static_table_v18_normal.emplace_back( 2.10, 2.101906, 0.055163 );
    M_static_table_v18_normal.emplace_back( 2.20, 2.206711, 0.049643 );
    M_static_table_v18_normal.emplace_back( 2.30, 2.299464, 0.043110 );
    M_static_table_v18_normal.emplace_back( 2.40, 2.396485, 0.053912 );
    M_static_table_v18_normal.emplace_back( 2.50, 2.497214, 0.046818 );
    M_static_table_v18_normal.emplace_back( 2.60, 2.592638, 0.048606 );
    M_static_table_v18_normal.emplace_back( 2.70, 2.691708, 0.050464 );
    M_static_table_v18_normal.emplace_back( 2.80, 2.794564, 0.052392 );
    M_static_table_v18_normal.emplace_back( 2.90, 2.901349, 0.054394 );
    M_static_table_v18_normal.emplace_back( 3.00, 3.000751, 0.045008 );
    M_static_table_v18_normal.emplace_back( 3.10, 3.092138, 0.046379 );
    M_static_table_v18_normal.emplace_back( 3.20, 3.198481, 0.059965 );
    M_static_table_v18_normal.emplace_back( 3.30, 3.308063, 0.049618 );
    M_static_table_v18_normal.emplace_back( 3.40, 3.408808, 0.051128 );
    M_static_table_v18_normal.emplace_back( 3.50, 3.499302, 0.039366 );
    M_static_table_v18_normal.emplace_back( 3.60, 3.592552, 0.053885 );
    M_static_table_v18_normal.emplace_back( 3.70, 3.701961, 0.055525 );
    M_static_table_v18_normal.emplace_back( 3.80, 3.800237, 0.042751 );
    M_static_table_v18_normal.emplace_back( 3.90, 3.901506, 0.058518 );
    M_static_table_v18_normal.emplace_back( 4.00, 4.005080, 0.045056 );
    M_static_table_v18_normal.emplace_back( 4.10, 4.096216, 0.046081 );
    M_static_table_v18_normal.emplace_back( 4.20, 4.189425, 0.047129 );
    M_static_table_v18_normal.emplace_back( 4.30, 4.301065, 0.064511 );
    M_static_table_v18_normal.emplace_back( 4.40, 4.415246, 0.049670 );
    M_static_table_v18_normal.emplace_back( 4.50, 4.515715, 0.050800 );
    M_static_table_v18_normal.emplace_back( 4.60, 4.601021, 0.034507 );
    M_static_table_v18_normal.emplace_back( 4.70, 4.688269, 0.052741 );
    M_static_table_v18_normal.emplace_back( 4.80, 4.794951, 0.053941 );
    M_static_table_v18_normal.emplace_back( 4.90, 4.904060, 0.055169 );
    M_static_table_v18_normal.emplace_back( 5.00, 4.996703, 0.037475 );
    M_static_table_v18_normal.emplace_back( 5.10, 5.091454, 0.057277 );
    M_static_table_v18_normal.emplace_back( 5.20, 5.207310, 0.058580 );
    M_static_table_v18_normal.emplace_back( 5.30, 5.305682, 0.039792 );
    M_static_table_v18_normal.emplace_back( 5.40, 5.406292, 0.060819 );
    M_static_table_v18_normal.emplace_back( 5.50, 5.508423, 0.041313 );
    M_static_table_v18_normal.emplace_back( 5.60, 5.591672, 0.041937 );
    M_static_table_v18_normal.emplace_back( 5.70, 5.697705, 0.064097 );
    M_static_table_v18_normal.emplace_back( 5.80, 5.805340, 0.043539 );
    M_static_table_v18_normal.emplace_back( 5.90, 5.893077, 0.044198 );
    M_static_table_v18_normal.emplace_back( 6.00, 6.004826, 0.067552 );
    M_static_table_v18_normal.emplace_back( 6.10, 6.118263, 0.045886 );
    M_static_table_v18_normal.emplace_back( 6.20, 6.210729, 0.046580 );
    M_static_table_v18_normal.emplace_back( 6.30, 6.304592, 0.047284 );
    M_static_table_v18_normal.emplace_back( 6.40, 6.399874, 0.047999 );
    M_static_table_v18_normal.emplace_back( 6.50, 6.496596, 0.048724 );
    M_static_table_v18_normal.emplace_back( 6.60, 6.594779, 0.049460 );
    M_static_table_v18_normal.emplace_back( 6.70, 6.694446, 0.050208 );
    M_static_table_v18_normal.emplace_back( 6.80, 6.795620, 0.050967 );
    M_static_table_v18_normal.emplace_back( 6.90, 6.898322, 0.051736 );
    M_static_table_v18_normal.emplace_back( 7.00, 7.002577, 0.052519 );
    M_static_table_v18_normal.emplace_back( 7.10, 7.108407, 0.053312 );
    M_static_table_v18_normal.emplace_back( 7.20, 7.215837, 0.054118 );
    M_static_table_v18_normal.emplace_back( 7.30, 7.297320, 0.027365 );
    M_static_table_v18_normal.emplace_back( 7.40, 7.380034, 0.055350 );
    M_static_table_v18_normal.emplace_back( 7.50, 7.491569, 0.056186 );
    M_static_table_v18_normal.emplace_back( 7.60, 7.604789, 0.057035 );
    M_static_table_v18_normal.emplace_back( 7.70, 7.719721, 0.057897 );
    M_static_table_v18_normal.emplace_back( 7.80, 7.806894, 0.029276 );
    M_static_table_v18_normal.emplace_back( 7.90, 7.895383, 0.059214 );
    M_static_table_v18_normal.emplace_back( 8.00, 8.014707, 0.060109 );
    M_static_table_v18_normal.emplace_back( 8.10, 8.105211, 0.030394 );
    M_static_table_v18_normal.emplace_back( 8.20, 8.197082, 0.061477 );
    M_static_table_v18_normal.emplace_back( 8.30, 8.289645, 0.031086 );
    M_static_table_v18_normal.emplace_back( 8.40, 8.383607, 0.062875 );
    M_static_table_v18_normal.emplace_back( 8.50, 8.510309, 0.063826 );
    M_static_table_v18_normal.emplace_back( 8.60, 8.606409, 0.032273 );
    M_static_table_v18_normal.emplace_back( 8.70, 8.703961, 0.065278 );
    M_static_table_v18_normal.emplace_back( 8.80, 8.802248, 0.033008 );
    M_static_table_v18_normal.emplace_back( 8.90, 8.902020, 0.066764 );
    M_static_table_v18_normal.emplace_back( 9.00, 9.002543, 0.033759 );
    M_static_table_v18_normal.emplace_back( 9.10, 9.104585, 0.068283 );
    M_static_table_v18_normal.emplace_back( 9.20, 9.207396, 0.034527 );
    M_static_table_v18_normal.emplace_back( 9.30, 9.311760, 0.069836 );
    M_static_table_v18_normal.emplace_back( 9.40, 9.416910, 0.035313 );
    M_static_table_v18_normal.emplace_back( 9.50, 9.487803, 0.035579 );
    M_static_table_v18_normal.emplace_back( 9.60, 9.595346, 0.071964 );
    M_static_table_v18_normal.emplace_back( 9.70, 9.703698, 0.036388 );
    M_static_table_v18_normal.emplace_back( 9.80, 9.776749, 0.036662 );
    M_static_table_v18_normal.emplace_back( 9.90, 9.887567, 0.074155 );
    M_static_table_v18_normal.emplace_back( 10.00, 9.999220, 0.037497 );
    M_static_table_v18_normal.emplace_back( 10.10, 10.074496, 0.037779 );
    M_static_table_v18_normal.emplace_back( 10.20, 10.188689, 0.076414 );
    M_static_table_v18_normal.emplace_back( 10.30, 10.303741, 0.038638 );
    M_static_table_v18_normal.emplace_back( 10.40, 10.381310, 0.038930 );
    M_static_table_v18_normal.emplace_back( 10.50, 10.498981, 0.078741 );
    M_static_table_v18_normal.emplace_back( 10.60, 10.617537, 0.039815 );
    M_static_table_v18_normal.emplace_back( 10.70, 10.697468, 0.040115 );
    M_static_table_v18_normal.emplace_back( 10.80, 10.778000, 0.040417 );
    M_static_table_v18_normal.emplace_back( 10.90, 10.900167, 0.081749 );
    M_static_table_v18_normal.emplace_back( 11.00, 11.023254, 0.041337 );
    M_static_table_v18_normal.emplace_back( 11.10, 11.106239, 0.041648 );
    M_static_table_v18_normal.emplace_back( 11.20, 11.189849, 0.041961 );
    M_static_table_v18_normal.emplace_back( 11.30, 11.274089, 0.042278 );
    M_static_table_v18_normal.emplace_back( 11.40, 11.401879, 0.085512 );
    M_static_table_v18_normal.emplace_back( 11.50, 11.530631, 0.043239 );
    M_static_table_v18_normal.emplace_back( 11.60, 11.617436, 0.043565 );
    M_static_table_v18_normal.emplace_back( 11.70, 11.704894, 0.043893 );
    M_static_table_v18_normal.emplace_back( 11.80, 11.793011, 0.044224 );
    M_static_table_v18_normal.emplace_back( 11.90, 11.881791, 0.044556 );
    M_static_table_v18_normal.emplace_back( 12.00, 11.971240, 0.044892 );
    M_static_table_v18_normal.emplace_back( 12.10, 12.061361, 0.045229 );
    M_static_table_v18_normal.emplace_back( 12.20, 12.198075, 0.091484 );
    M_static_table_v18_normal.emplace_back( 12.30, 12.335818, 0.046259 );
    M_static_table_v18_normal.emplace_back( 12.40, 12.428684, 0.046607 );
    M_static_table_v18_normal.emplace_back( 12.50, 12.522250, 0.046958 );
    M_static_table_v18_normal.emplace_back( 12.60, 12.616520, 0.047312 );
    M_static_table_v18_normal.emplace_back( 12.70, 12.711500, 0.047668 );
    M_static_table_v18_normal.emplace_back( 12.80, 12.807194, 0.048026 );
    M_static_table_v18_normal.emplace_back( 12.90, 12.903609, 0.048388 );
    M_static_table_v18_normal.emplace_back( 13.00, 13.000750, 0.048752 );
    M_static_table_v18_normal.emplace_back( 13.10, 13.098622, 0.049119 );
    M_static_table_v18_normal.emplace_back( 13.20, 13.197231, 0.049489 );
    M_static_table_v18_normal.emplace_back( 13.30, 13.296583, 0.049862 );
    M_static_table_v18_normal.emplace_back( 13.40, 13.396682, 0.050237 );
    M_static_table_v18_normal.emplace_back( 13.50, 13.497534, 0.050615 );
    M_static_table_v18_normal.emplace_back( 13.60, 13.599147, 0.050997 );
    M_static_table_v18_normal.emplace_back( 13.70, 13.701524, 0.051380 );
    M_static_table_v18_normal.emplace_back( 13.80, 13.804671, 0.051767 );
    M_static_table_v18_normal.emplace_back( 13.90, 13.908596, 0.052157 );
    M_static_table_v18_normal.emplace_back( 14.00, 14.013302, 0.052549 );
    M_static_table_v18_normal.emplace_back( 14.10, 14.118797, 0.052945 );
    M_static_table_v18_normal.emplace_back( 14.20, 14.225086, 0.053343 );
    M_static_table_v18_normal.emplace_back( 14.30, 14.332175, 0.053745 );
    M_static_table_v18_normal.emplace_back( 14.40, 14.440071, 0.054150 );
    M_static_table_v18_normal.emplace_back( 14.50, 14.548779, 0.054558 );
    M_static_table_v18_normal.emplace_back( 14.70, 14.658305, 0.054968 );
    M_static_table_v18_normal.emplace_back( 14.80, 14.768655, 0.055382 );
    M_static_table_v18_normal.emplace_back( 14.90, 14.879837, 0.055799 );
    M_static_table_v18_normal.emplace_back( 15.00, 14.991855, 0.056219 );
    M_static_table_v18_normal.emplace_back( 15.10, 15.104716, 0.056642 );
    M_static_table_v18_normal.emplace_back( 15.20, 15.218428, 0.057069 );
    M_static_table_v18_normal.emplace_back( 15.30, 15.332995, 0.057498 );
    M_static_table_v18_normal.emplace_back( 15.40, 15.448425, 0.057931 );
    M_static_table_v18_normal.emplace_back( 15.60, 15.564723, 0.058367 );
    M_static_table_v18_normal.emplace_back( 15.70, 15.681898, 0.058807 );
    M_static_table_v18_normal.emplace_back( 15.80, 15.799954, 0.059249 );
    M_static_table_v18_normal.emplace_back( 15.90, 15.918899, 0.059695 );
    M_static_table_v18_normal.emplace_back( 16.00, 16.038740, 0.060145 );
    M_static_table_v18_normal.emplace_back( 16.20, 16.159483, 0.060598 );
    M_static_table_v18_normal.emplace_back( 16.30, 16.281135, 0.061054 );
    M_static_table_v18_normal.emplace_back( 16.40, 16.403702, 0.061514 );
    M_static_table_v18_normal.emplace_back( 16.50, 16.527192, 0.061977 );
    M_static_table_v18_normal.emplace_back( 16.70, 16.651612, 0.062444 );
    M_static_table_v18_normal.emplace_back( 16.80, 16.776969, 0.062913 );
    M_static_table_v18_normal.emplace_back( 16.90, 16.903269, 0.063387 );
    M_static_table_v18_normal.emplace_back( 17.00, 17.030520, 0.063865 );
    M_static_table_v18_normal.emplace_back( 17.20, 17.158730, 0.064345 );
    M_static_table_v18_normal.emplace_back( 17.30, 17.287904, 0.064829 );
    M_static_table_v18_normal.emplace_back( 17.40, 17.418050, 0.065318 );
    M_static_table_v18_normal.emplace_back( 17.50, 17.549177, 0.065809 );
    M_static_table_v18_normal.emplace_back( 17.70, 17.681290, 0.066305 );
    M_static_table_v18_normal.emplace_back( 17.80, 17.814399, 0.066804 );
    M_static_table_v18_normal.emplace_back( 17.90, 17.948509, 0.067307 );
    M_static_table_v18_normal.emplace_back( 18.10, 18.083629, 0.067813 );
    M_static_table_v18_normal.emplace_back( 18.20, 18.219766, 0.068324 );
    M_static_table_v18_normal.emplace_back( 18.40, 18.356928, 0.068838 );
    M_static_table_v18_normal.emplace_back( 18.50, 18.495122, 0.069357 );
    M_static_table_v18_normal.emplace_back( 18.60, 18.634357, 0.069879 );
    M_static_table_v18_normal.emplace_back( 18.80, 18.774640, 0.070405 );
    M_static_table_v18_normal.emplace_back( 18.90, 18.915979, 0.070935 );
    M_static_table_v18_normal.emplace_back( 19.10, 19.058383, 0.071469 );
    M_static_table_v18_normal.emplace_back( 19.20, 19.201858, 0.072007 );
    M_static_table_v18_normal.emplace_back( 19.30, 19.346413, 0.072549 );
    M_static_table_v18_normal.emplace_back( 19.50, 19.492057, 0.073095 );
    M_static_table_v18_normal.emplace_back( 19.60, 19.638797, 0.073645 );
    M_static_table_v18_normal.emplace_back( 19.80, 19.786641, 0.074200 );
    M_static_table_v18_normal.emplace_back( 19.90, 19.935599, 0.074759 );
    M_static_table_v18_normal.emplace_back( 20.10, 20.085679, 0.075321 );
    M_static_table_v18_normal.emplace_back( 20.20, 20.236888, 0.075888 );
    M_static_table_v18_normal.emplace_back( 20.40, 20.389235, 0.076459 );
    M_static_table_v18_normal.emplace_back( 20.50, 20.542729, 0.077035 );
    M_static_table_v18_normal.emplace_back( 20.70, 20.697378, 0.077615 );
    M_static_table_v18_normal.emplace_back( 20.90, 20.853192, 0.078200 );
    M_static_table_v18_normal.emplace_back( 21.00, 21.010179, 0.078788 );
    M_static_table_v18_normal.emplace_back( 21.20, 21.168348, 0.079381 );
    M_static_table_v18_normal.emplace_back( 21.30, 21.327708, 0.079979 );
    M_static_table_v18_normal.emplace_back( 21.50, 21.488267, 0.080581 );
    M_static_table_v18_normal.emplace_back( 21.60, 21.650035, 0.081188 );
    M_static_table_v18_normal.emplace_back( 21.80, 21.813021, 0.081799 );
    M_static_table_v18_normal.emplace_back( 22.00, 21.977233, 0.082414 );
    M_static_table_v18_normal.emplace_back( 22.10, 22.142682, 0.083035 );
    M_static_table_v18_normal.emplace_back( 22.30, 22.309377, 0.083660 );
    M_static_table_v18_normal.emplace_back( 22.50, 22.477326, 0.084290 );
    M_static_table_v18_normal.emplace_back( 22.60, 22.646539, 0.084924 );
    M_static_table_v18_normal.emplace_back( 22.80, 22.817027, 0.085564 );
    M_static_table_v18_normal.emplace_back( 23.00, 22.988798, 0.086208 );
    M_static_table_v18_normal.emplace_back( 23.20, 23.161863, 0.086857 );
    M_static_table_v18_normal.emplace_back( 23.30, 23.336230, 0.087511 );
    M_static_table_v18_normal.emplace_back( 23.50, 23.511909, 0.088169 );
    M_static_table_v18_normal.emplace_back( 23.70, 23.688911, 0.088833 );
    M_static_table_v18_normal.emplace_back( 23.90, 23.867246, 0.089502 );
    M_static_table_v18_normal.emplace_back( 24.00, 24.046924, 0.090176 );
    M_static_table_v18_normal.emplace_back( 24.20, 24.227953, 0.090854 );
    M_static_table_v18_normal.emplace_back( 24.40, 24.410346, 0.091539 );
    M_static_table_v18_normal.emplace_back( 24.60, 24.594112, 0.092228 );
    M_static_table_v18_normal.emplace_back( 24.80, 24.779261, 0.092922 );
    M_static_table_v18_normal.emplace_back( 25.00, 24.965804, 0.093621 );
    M_static_table_v18_normal.emplace_back( 25.20, 25.153752, 0.094327 );
    M_static_table_v18_normal.emplace_back( 25.30, 25.343114, 0.095036 );
    M_static_table_v18_normal.emplace_back( 25.50, 25.533902, 0.095752 );
    M_static_table_v18_normal.emplace_back( 25.70, 25.726126, 0.096473 );
    M_static_table_v18_normal.emplace_back( 25.90, 25.919797, 0.097199 );
    M_static_table_v18_normal.emplace_back( 26.10, 26.114927, 0.097931 );
    M_static_table_v18_normal.emplace_back( 26.30, 26.311525, 0.098668 );
    M_static_table_v18_normal.emplace_back( 26.50, 26.509604, 0.099411 );
    M_static_table_v18_normal.emplace_back( 26.70, 26.709173, 0.100159 );
    M_static_table_v18_normal.emplace_back( 26.90, 26.910245, 0.100913 );
    M_static_table_v18_normal.emplace_back( 27.10, 27.112830, 0.101673 );
    M_static_table_v18_normal.emplace_back( 27.30, 27.316941, 0.102438 );
    M_static_table_v18_normal.emplace_back( 27.50, 27.522588, 0.103209 );
    M_static_table_v18_normal.emplace_back( 27.70, 27.729784, 0.103987 );
    M_static_table_v18_normal.emplace_back( 27.90, 27.938539, 0.104769 );
    M_static_table_v18_normal.emplace_back( 28.10, 28.148866, 0.105558 );
    M_static_table_v18_normal.emplace_back( 28.40, 28.360776, 0.106353 );
    M_static_table_v18_normal.emplace_back( 28.60, 28.574281, 0.107153 );
    M_static_table_v18_normal.emplace_back( 28.80, 28.789394, 0.107960 );
    M_static_table_v18_normal.emplace_back( 29.00, 29.006126, 0.108772 );
    M_static_table_v18_normal.emplace_back( 29.20, 29.224490, 0.109592 );
    M_static_table_v18_normal.emplace_back( 29.40, 29.444498, 0.110417 );
    M_static_table_v18_normal.emplace_back( 29.70, 29.666162, 0.111248 );
    M_static_table_v18_normal.emplace_back( 29.90, 29.889494, 0.112085 );
    M_static_table_v18_normal.emplace_back( 30.10, 30.114508, 0.112929 );
    M_static_table_v18_normal.emplace_back( 30.30, 30.341216, 0.113779 );
    M_static_table_v18_normal.emplace_back( 30.60, 30.569631, 0.114636 );
    M_static_table_v18_normal.emplace_back( 30.80, 30.799765, 0.115499 );
    M_static_table_v18_normal.emplace_back( 31.00, 31.031631, 0.116368 );
    M_static_table_v18_normal.emplace_back( 31.30, 31.265244, 0.117245 );
    M_static_table_v18_normal.emplace_back( 31.50, 31.500615, 0.118127 );
    M_static_table_v18_normal.emplace_back( 31.70, 31.737757, 0.119016 );
    M_static_table_v18_normal.emplace_back( 32.00, 31.976685, 0.119912 );
    M_static_table_v18_normal.emplace_back( 32.20, 32.217412, 0.120815 );
    M_static_table_v18_normal.emplace_back( 32.50, 32.459951, 0.121724 );
    M_static_table_v18_normal.emplace_back( 32.70, 32.704316, 0.122640 );
    M_static_table_v18_normal.emplace_back( 33.00, 32.950520, 0.123564 );
    M_static_table_v18_normal.emplace_back( 33.20, 33.198578, 0.124494 );
    M_static_table_v18_normal.emplace_back( 33.40, 33.448504, 0.125431 );
    M_static_table_v18_normal.emplace_back( 33.70, 33.700311, 0.126375 );
    M_static_table_v18_normal.emplace_back( 34.00, 33.954013, 0.127327 );
    M_static_table_v18_normal.emplace_back( 34.20, 34.209626, 0.128285 );
    M_static_table_v18_normal.emplace_back( 34.50, 34.467162, 0.129251 );
    M_static_table_v18_normal.emplace_back( 34.70, 34.726638, 0.130224 );
    M_static_table_v18_normal.emplace_back( 35.00, 34.988067, 0.131204 );
    M_static_table_v18_normal.emplace_back( 35.30, 35.251464, 0.132192 );
    M_static_table_v18_normal.emplace_back( 35.50, 35.516844, 0.133187 );
    M_static_table_v18_normal.emplace_back( 35.80, 35.784221, 0.134190 );
    M_static_table_v18_normal.emplace_back( 36.10, 36.053612, 0.135200 );
    M_static_table_v18_normal.emplace_back( 36.30, 36.325031, 0.136218 );
    M_static_table_v18_normal.emplace_back( 36.60, 36.598492, 0.137243 );
    M_static_table_v18_normal.emplace_back( 36.90, 36.874013, 0.138277 );
    M_static_table_v18_normal.emplace_back( 37.20, 37.151608, 0.139318 );
    M_static_table_v18_normal.emplace_back( 37.40, 37.431292, 0.140366 );
    M_static_table_v18_normal.emplace_back( 37.70, 37.713082, 0.141423 );
    M_static_table_v18_normal.emplace_back( 38.00, 37.996994, 0.142488 );
    M_static_table_v18_normal.emplace_back( 38.30, 38.283042, 0.143560 );
    M_static_table_v18_normal.emplace_back( 38.60, 38.571244, 0.144641 );
    M_static_table_v18_normal.emplace_back( 38.90, 38.861616, 0.145730 );
    M_static_table_v18_normal.emplace_back( 39.20, 39.154174, 0.146827 );
    M_static_table_v18_normal.emplace_back( 39.40, 39.448935, 0.147933 );
    M_static_table_v18_normal.emplace_back( 39.70, 39.745914, 0.149046 );
    M_static_table_v18_normal.emplace_back( 40.00, 40.045129, 0.150168 );
    M_static_table_v18_normal.emplace_back( 40.30, 40.346597, 0.151299 );
    M_static_table_v18_normal.emplace_back( 40.70, 40.650334, 0.152438 );
    M_static_table_v18_normal.emplace_back( 41.00, 40.956357, 0.153585 );
    M_static_table_v18_normal.emplace_back( 41.30, 41.264685, 0.154742 );
    M_static_table_v18_normal.emplace_back( 41.60, 41.575333, 0.155906 );
    M_static_table_v18_normal.emplace_back( 41.90, 41.888320, 0.157080 );
    M_static_table_v18_normal.emplace_back( 42.20, 42.203664, 0.158263 );
    M_static_table_v18_normal.emplace_back( 42.50, 42.521381, 0.159454 );
    M_static_table_v18_normal.emplace_back( 42.80, 42.841491, 0.160655 );
    M_static_table_v18_normal.emplace_back( 43.20, 43.164010, 0.161864 );
    M_static_table_v18_normal.emplace_back( 43.50, 43.488957, 0.163083 );
    M_static_table_v18_normal.emplace_back( 43.80, 43.816350, 0.164310 );
    M_static_table_v18_normal.emplace_back( 44.10, 44.146208, 0.165547 );
    M_static_table_v18_normal.emplace_back( 44.50, 44.478549, 0.166793 );
    M_static_table_v18_normal.emplace_back( 44.80, 44.813392, 0.168049 );
    M_static_table_v18_normal.emplace_back( 45.20, 45.150756, 0.169314 );
    M_static_table_v18_normal.emplace_back( 45.50, 45.490660, 0.170589 );
    M_static_table_v18_normal.emplace_back( 45.80, 45.833123, 0.171873 );
    M_static_table_v18_normal.emplace_back( 46.20, 46.178164, 0.173167 );
    M_static_table_v18_normal.emplace_back( 46.50, 46.525802, 0.174471 );
    M_static_table_v18_normal.emplace_back( 46.90, 46.876057, 0.175784 );
    M_static_table_v18_normal.emplace_back( 47.20, 47.228949, 0.177107 );
    M_static_table_v18_normal.emplace_back( 47.60, 47.584498, 0.178441 );
    M_static_table_v18_normal.emplace_back( 47.90, 47.942723, 0.179784 );
    M_static_table_v18_normal.emplace_back( 48.30, 48.303646, 0.181138 );
    M_static_table_v18_normal.emplace_back( 48.70, 48.667285, 0.182501 );
    M_static_table_v18_normal.emplace_back( 49.00, 49.033662, 0.183875 );
    M_static_table_v18_normal.emplace_back( 49.40, 49.402797, 0.185260 );
    M_static_table_v18_normal.emplace_back( 49.80, 49.774711, 0.186654 );
    M_static_table_v18_normal.emplace_back( 50.10, 50.149424, 0.188059 );
    M_static_table_v18_normal.emplace_back( 50.50, 50.526959, 0.189475 );
    M_static_table_v18_normal.emplace_back( 50.90, 50.907336, 0.190901 );
    M_static_table_v18_normal.emplace_back( 51.30, 51.290576, 0.192338 );
    M_static_table_v18_normal.emplace_back( 51.70, 51.676702, 0.193787 );
    M_static_table_v18_normal.emplace_back( 52.10, 52.065734, 0.195245 );
    M_static_table_v18_normal.emplace_back( 52.50, 52.457695, 0.196715 );
    M_static_table_v18_normal.emplace_back( 52.90, 52.852607, 0.198196 );
    M_static_table_v18_normal.emplace_back( 53.30, 53.250491, 0.199688 );
    M_static_table_v18_normal.emplace_back( 53.70, 53.651372, 0.201192 );
    M_static_table_v18_normal.emplace_back( 54.10, 54.055270, 0.202706 );
    M_static_table_v18_normal.emplace_back( 54.50, 54.462208, 0.204232 );
    M_static_table_v18_normal.emplace_back( 54.90, 54.872211, 0.205770 );
    M_static_table_v18_normal.emplace_back( 55.30, 55.285299, 0.207318 );
    M_static_table_v18_normal.emplace_back( 55.70, 55.701498, 0.208880 );
    M_static_table_v18_normal.emplace_back( 56.10, 56.120830, 0.210452 );
    M_static_table_v18_normal.emplace_back( 56.50, 56.543318, 0.212036 );
    M_static_table_v18_normal.emplace_back( 57.00, 56.968987, 0.213632 );
    M_static_table_v18_normal.emplace_back( 57.40, 57.397861, 0.215241 );
    M_static_table_v18_normal.emplace_back( 57.80, 57.829963, 0.216861 );
    M_static_table_v18_normal.emplace_back( 58.30, 58.265319, 0.218494 );
    M_static_table_v18_normal.emplace_back( 58.70, 58.703951, 0.220138 );
    M_static_table_v18_normal.emplace_back( 59.10, 59.145886, 0.221796 );
    M_static_table_v18_normal.emplace_back( 59.60, 59.591148, 0.223466 );
    M_static_table_v18_normal.emplace_back( 60.00, 60.039762, 0.225148 );
    M_static_table_v18_normal.emplace_back( 60.50, 60.491753, 0.226843 );
    M_static_table_v18_normal.emplace_back( 60.90, 60.947146, 0.228550 );
    M_static_table_v18_normal.emplace_back( 61.40, 61.405968, 0.230271 );
    M_static_table_v18_normal.emplace_back( 61.90, 61.868245, 0.232005 );
    M_static_table_v18_normal.emplace_back( 62.30, 62.334001, 0.233751 );
    M_static_table_v18_normal.emplace_back( 62.80, 62.803263, 0.235511 );
    M_static_table_v18_normal.emplace_back( 63.30, 63.276059, 0.237284 );
    M_static_table_v18_normal.emplace_back( 63.80, 63.752413, 0.239070 );
    M_static_table_v18_normal.emplace_back( 64.20, 64.232354, 0.240870 );
    M_static_table_v18_normal.emplace_back( 64.70, 64.715907, 0.242683 );
    M_static_table_v18_normal.emplace_back( 65.20, 65.203101, 0.244510 );
    M_static_table_v18_normal.emplace_back( 65.70, 65.693963, 0.246351 );
    M_static_table_v18_normal.emplace_back( 66.20, 66.188520, 0.248205 );
    M_static_table_v18_normal.emplace_back( 66.70, 66.686800, 0.250074 );
    M_static_table_v18_normal.emplace_back( 67.20, 67.188832, 0.251957 );
    M_static_table_v18_normal.emplace_back( 67.70, 67.694642, 0.253853 );
    M_static_table_v18_normal.emplace_back( 68.20, 68.204261, 0.255765 );
    M_static_table_v18_normal.emplace_back( 68.70, 68.717716, 0.257690 );
    M_static_table_v18_normal.emplace_back( 69.20, 69.235036, 0.259630 );
    M_static_table_v18_normal.emplace_back( 69.80, 69.756251, 0.261585 );
    M_static_table_v18_normal.emplace_back( 70.30, 70.281390, 0.263554 );
    M_static_table_v18_normal.emplace_back( 70.80, 70.810482, 0.265538 );
    M_static_table_v18_normal.emplace_back( 71.30, 71.343557, 0.267537 );
    M_static_table_v18_normal.emplace_back( 71.90, 71.880645, 0.269551 );
    M_static_table_v18_normal.emplace_back( 72.40, 72.421776, 0.271580 );
    M_static_table_v18_normal.emplace_back( 73.00, 72.966982, 0.273625 );
    M_static_table_v18_normal.emplace_back( 73.50, 73.516292, 0.275685 );
    M_static_table_v18_normal.emplace_back( 74.10, 74.069737, 0.277760 );
    M_static_table_v18_normal.emplace_back( 74.60, 74.627348, 0.279851 );
    M_static_table_v18_normal.emplace_back( 75.20, 75.189158, 0.281958 );
    M_static_table_v18_normal.emplace_back( 75.80, 75.755196, 0.284080 );
    M_static_table_v18_normal.emplace_back( 76.30, 76.325496, 0.286219 );
    M_static_table_v18_normal.emplace_back( 76.90, 76.900089, 0.288374 );
    M_static_table_v18_normal.emplace_back( 77.50, 77.479008, 0.290545 );
    M_static_table_v18_normal.emplace_back( 78.10, 78.062285, 0.292732 );
    M_static_table_v18_normal.emplace_back( 78.60, 78.649953, 0.294935 );
    M_static_table_v18_normal.emplace_back( 79.20, 79.242045, 0.297156 );
    M_static_table_v18_normal.emplace_back( 79.80, 79.838595, 0.299393 );
    M_static_table_v18_normal.emplace_back( 80.40, 80.439636, 0.301647 );
    M_static_table_v18_normal.emplace_back( 81.00, 81.045201, 0.303918 );
    M_static_table_v18_normal.emplace_back( 81.70, 81.655325, 0.306206 );
    M_static_table_v18_normal.emplace_back( 82.30, 82.270042, 0.308511 );
    M_static_table_v18_normal.emplace_back( 82.90, 82.889387, 0.310833 );
    M_static_table_v18_normal.emplace_back( 83.50, 83.513395, 0.313174 );
    M_static_table_v18_normal.emplace_back( 84.10, 84.142100, 0.315531 );
    M_static_table_v18_normal.emplace_back( 84.80, 84.775538, 0.317906 );
    M_static_table_v18_normal.emplace_back( 85.40, 85.413745, 0.320300 );
    M_static_table_v18_normal.emplace_back( 86.10, 86.056756, 0.322711 );
    M_static_table_v18_normal.emplace_back( 86.70, 86.704608, 0.325140 );
    M_static_table_v18_normal.emplace_back( 87.40, 87.357337, 0.327588 );
    M_static_table_v18_normal.emplace_back( 88.00, 88.014981, 0.330055 );
    M_static_table_v18_normal.emplace_back( 88.70, 88.677575, 0.332539 );
    M_static_table_v18_normal.emplace_back( 89.30, 89.345157, 0.335043 );
    M_static_table_v18_normal.emplace_back( 90.00, 90.017765, 0.337565 );
    M_static_table_v18_normal.emplace_back( 90.70, 90.695436, 0.340106 );
    M_static_table_v18_normal.emplace_back( 91.40, 91.378209, 0.342667 );
    M_static_table_v18_normal.emplace_back( 92.10, 92.066122, 0.345246 );
    M_static_table_v18_normal.emplace_back( 92.80, 92.759214, 0.347845 );
    M_static_table_v18_normal.emplace_back( 93.50, 93.457523, 0.350464 );
    M_static_table_v18_normal.emplace_back( 94.20, 94.161089, 0.353102 );
    M_static_table_v18_normal.emplace_back( 94.90, 94.869953, 0.355761 );
    M_static_table_v18_normal.emplace_back( 95.60, 95.584152, 0.358438 );
    M_static_table_v18_normal.emplace_back( 96.30, 96.303728, 0.361137 );
    M_static_table_v18_normal.emplace_back( 97.00, 97.028722, 0.363856 );
    M_static_table_v18_normal.emplace_back( 97.80, 97.759173, 0.366595 );
    M_static_table_v18_normal.emplace_back( 98.50, 98.495123, 0.369355 );
    M_static_table_v18_normal.emplace_back( 99.20, 99.236613, 0.372135 );
    M_static_table_v18_normal.emplace_back( 100.00, 99.983686, 0.374937 );
    M_static_table_v18_normal.emplace_back( 100.70, 100.736383, 0.377759 );
    M_static_table_v18_normal.emplace_back( 101.50, 101.494746, 0.380603 );
    M_static_table_v18_normal.emplace_back( 102.30, 102.258818, 0.383468 );
    M_static_table_v18_normal.emplace_back( 103.00, 103.028643, 0.386356 );
    M_static_table_v18_normal.emplace_back( 103.80, 103.804263, 0.389264 );
    M_static_table_v18_normal.emplace_back( 104.60, 104.585721, 0.392194 );
    M_static_table_v18_normal.emplace_back( 105.40, 105.373063, 0.395147 );
    M_static_table_v18_normal.emplace_back( 106.20, 106.166332, 0.398122 );
    M_static_table_v18_normal.emplace_back( 107.00, 106.965573, 0.401119 );
    M_static_table_v18_normal.emplace_back( 107.80, 107.770831, 0.404139 );
    M_static_table_v18_normal.emplace_back( 108.60, 108.582151, 0.407181 );
    M_static_table_v18_normal.emplace_back( 109.40, 109.399578, 0.410246 );
    M_static_table_v18_normal.emplace_back( 110.20, 110.223160, 0.413335 );
    M_static_table_v18_normal.emplace_back( 111.10, 111.052941, 0.416446 );
    M_static_table_v18_normal.emplace_back( 111.90, 111.888969, 0.419581 );
    M_static_table_v18_normal.emplace_back( 112.70, 112.731291, 0.422740 );
    M_static_table_v18_normal.emplace_back( 113.60, 113.579955, 0.425923 );
    M_static_table_v18_normal.emplace_back( 114.40, 114.435007, 0.429129 );
    M_static_table_v18_normal.emplace_back( 115.30, 115.296496, 0.432360 );
    M_static_table_v18_normal.emplace_back( 116.20, 116.164470, 0.435614 );
    M_static_table_v18_normal.emplace_back( 117.00, 117.038979, 0.438894 );
    M_static_table_v18_normal.emplace_back( 117.90, 117.920072, 0.442198 );
    M_static_table_v18_normal.emplace_back( 118.80, 118.807797, 0.445527 );
    M_static_table_v18_normal.emplace_back( 119.70, 119.702205, 0.448881 );
    M_static_table_v18_normal.emplace_back( 120.60, 120.603347, 0.452260 );
    M_static_table_v18_normal.emplace_back( 121.50, 121.511272, 0.455665 );
    M_static_table_v18_normal.emplace_back( 122.40, 122.426033, 0.459095 );
    M_static_table_v18_normal.emplace_back( 123.30, 123.347680, 0.462551 );
    M_static_table_v18_normal.emplace_back( 124.30, 124.276266, 0.466034 );
    M_static_table_v18_normal.emplace_back( 125.20, 125.211842, 0.469542 );
    M_static_table_v18_normal.emplace_back( 126.20, 126.154461, 0.473077 );
    M_static_table_v18_normal.emplace_back( 127.10, 127.104176, 0.476638 );
    M_static_table_v18_normal.emplace_back( 128.10, 128.061041, 0.480226 );
    M_static_table_v18_normal.emplace_back( 129.00, 129.025110, 0.483842 );
    M_static_table_v18_normal.emplace_back( 130.00, 129.996436, 0.487484 );
    M_static_table_v18_normal.emplace_back( 131.00, 130.975075, 0.491154 );
    M_static_table_v18_normal.emplace_back( 132.00, 131.961081, 0.494852 );
    M_static_table_v18_normal.emplace_back( 133.00, 132.954510, 0.498577 );
    M_static_table_v18_normal.emplace_back( 134.00, 133.955417, 0.502330 );
    M_static_table_v18_normal.emplace_back( 135.00, 134.963860, 0.506112 );
    M_static_table_v18_normal.emplace_back( 136.00, 135.979894, 0.509922 );
    M_static_table_v18_normal.emplace_back( 137.00, 137.003577, 0.513761 );
    M_static_table_v18_normal.emplace_back( 138.00, 138.034967, 0.517629 );
    M_static_table_v18_normal.emplace_back( 139.10, 139.074121, 0.521525 );
    M_static_table_v18_normal.emplace_back( 140.10, 140.121098, 0.525451 );
    M_static_table_v18_normal.emplace_back( 141.20, 141.175957, 0.529407 );
    M_static_table_v18_normal.emplace_back( 142.20, 142.238757, 0.533392 );
    M_static_table_v18_normal.emplace_back( 143.30, 143.309558, 0.537408 );
    M_static_table_v18_normal.emplace_back( 144.40, 144.388421, 0.541454 );
    M_static_table_v18_normal.emplace_back( 145.50, 145.475405, 0.545530 );
    M_static_table_v18_normal.emplace_back( 146.60, 146.570573, 0.549637 );
    M_static_table_v18_normal.emplace_back( 147.70, 147.673984, 0.553774 );
    M_static_table_v18_normal.emplace_back( 148.80, 148.785703, 0.557944 );
    M_static_table_v18_normal.emplace_back( 149.90, 149.905791, 0.562144 );
    M_static_table_v18_normal.emplace_back( 151.00, 151.034311, 0.566376 );
    M_static_table_v18_normal.emplace_back( 152.20, 152.171327, 0.570640 );

    //
    M_static_table_v18_wide.emplace_back( 0.00, 0.025019, 0.025019 );
    M_static_table_v18_wide.emplace_back( 0.10, 0.100178, 0.050141 );
    M_static_table_v18_wide.emplace_back( 0.20, 0.200322, 0.050003 );
    M_static_table_v18_wide.emplace_back( 0.30, 0.301008, 0.050684 );
    M_static_table_v18_wide.emplace_back( 0.40, 0.401637, 0.049945 );
    M_static_table_v18_wide.emplace_back( 0.50, 0.501572, 0.049991 );
    M_static_table_v18_wide.emplace_back( 0.60, 0.599414, 0.047851 );
    M_static_table_v18_wide.emplace_back( 0.70, 0.699640, 0.052375 );
    M_static_table_v18_wide.emplace_back( 0.80, 0.799955, 0.047940 );
    M_static_table_v18_wide.emplace_back( 0.90, 0.897190, 0.049296 );
    M_static_table_v18_wide.emplace_back( 1.00, 0.996257, 0.049771 );
    M_static_table_v18_wide.emplace_back( 1.10, 1.095283, 0.049254 );
    M_static_table_v18_wide.emplace_back( 1.20, 1.198430, 0.053893 );
    M_static_table_v18_wide.emplace_back( 1.30, 1.304474, 0.052151 );
    M_static_table_v18_wide.emplace_back( 1.40, 1.405809, 0.049183 );
    M_static_table_v18_wide.emplace_back( 1.50, 1.499978, 0.044986 );
    M_static_table_v18_wide.emplace_back( 1.60, 1.600975, 0.056011 );
    M_static_table_v18_wide.emplace_back( 1.70, 1.699463, 0.042477 );
    M_static_table_v18_wide.emplace_back( 1.80, 1.795799, 0.053858 );
    M_static_table_v18_wide.emplace_back( 1.90, 1.897074, 0.047417 );
    M_static_table_v18_wide.emplace_back( 2.00, 1.994339, 0.049848 );
    M_static_table_v18_wide.emplace_back( 2.10, 2.096591, 0.052404 );
    M_static_table_v18_wide.emplace_back( 2.20, 2.204085, 0.055091 );
    M_static_table_v18_wide.emplace_back( 2.30, 2.305275, 0.046100 );
    M_static_table_v18_wide.emplace_back( 2.40, 2.399355, 0.047981 );
    M_static_table_v18_wide.emplace_back( 2.50, 2.497275, 0.049939 );
    M_static_table_v18_wide.emplace_back( 2.60, 2.599191, 0.051977 );
    M_static_table_v18_wide.emplace_back( 2.70, 2.705266, 0.054098 );
    M_static_table_v18_wide.emplace_back( 2.80, 2.801382, 0.042018 );
    M_static_table_v18_wide.emplace_back( 2.90, 2.901420, 0.058021 );
    M_static_table_v18_wide.emplace_back( 3.00, 3.004505, 0.045065 );
    M_static_table_v18_wide.emplace_back( 3.10, 3.096006, 0.046437 );
    M_static_table_v18_wide.emplace_back( 3.20, 3.190293, 0.047851 );
    M_static_table_v18_wide.emplace_back( 3.30, 3.287452, 0.049309 );
    M_static_table_v18_wide.emplace_back( 3.40, 3.387570, 0.050810 );
    M_static_table_v18_wide.emplace_back( 3.50, 3.490736, 0.052357 );
    M_static_table_v18_wide.emplace_back( 3.60, 3.597045, 0.053952 );
    M_static_table_v18_wide.emplace_back( 3.70, 3.706591, 0.055595 );
    M_static_table_v18_wide.emplace_back( 3.80, 3.800187, 0.038001 );
    M_static_table_v18_wide.emplace_back( 3.90, 3.896632, 0.058445 );
    M_static_table_v18_wide.emplace_back( 4.00, 3.995026, 0.039949 );
    M_static_table_v18_wide.emplace_back( 4.10, 4.096417, 0.061442 );
    M_static_table_v18_wide.emplace_back( 4.20, 4.199856, 0.041998 );
    M_static_table_v18_wide.emplace_back( 4.30, 4.306445, 0.064592 );
    M_static_table_v18_wide.emplace_back( 4.40, 4.415187, 0.044151 );
    M_static_table_v18_wide.emplace_back( 4.50, 4.504380, 0.045043 );
    M_static_table_v18_wide.emplace_back( 4.60, 4.595374, 0.045952 );
    M_static_table_v18_wide.emplace_back( 4.70, 4.688207, 0.046881 );
    M_static_table_v18_wide.emplace_back( 4.80, 4.782915, 0.047828 );
    M_static_table_v18_wide.emplace_back( 4.90, 4.879536, 0.048794 );
    M_static_table_v18_wide.emplace_back( 5.00, 4.978109, 0.049779 );
    M_static_table_v18_wide.emplace_back( 5.10, 5.078673, 0.050785 );
    M_static_table_v18_wide.emplace_back( 5.20, 5.181269, 0.051811 );
    M_static_table_v18_wide.emplace_back( 5.30, 5.285938, 0.052858 );
    M_static_table_v18_wide.emplace_back( 5.40, 5.392721, 0.053925 );
    M_static_table_v18_wide.emplace_back( 5.50, 5.501661, 0.055015 );
    M_static_table_v18_wide.emplace_back( 5.60, 5.612802, 0.056126 );
    M_static_table_v18_wide.emplace_back( 5.70, 5.697415, 0.028487 );
    M_static_table_v18_wide.emplace_back( 5.80, 5.783738, 0.057836 );
    M_static_table_v18_wide.emplace_back( 5.90, 5.900577, 0.059004 );
    M_static_table_v18_wide.emplace_back( 6.00, 6.019776, 0.060196 );
    M_static_table_v18_wide.emplace_back( 6.10, 6.110525, 0.030553 );
    M_static_table_v18_wide.emplace_back( 6.20, 6.203106, 0.062029 );
    M_static_table_v18_wide.emplace_back( 6.30, 6.296618, 0.031483 );
    M_static_table_v18_wide.emplace_back( 6.40, 6.392018, 0.063918 );
    M_static_table_v18_wide.emplace_back( 6.50, 6.488378, 0.032442 );
    M_static_table_v18_wide.emplace_back( 6.60, 6.586685, 0.065865 );
    M_static_table_v18_wide.emplace_back( 6.70, 6.685979, 0.033430 );
    M_static_table_v18_wide.emplace_back( 6.80, 6.787279, 0.067871 );
    M_static_table_v18_wide.emplace_back( 6.90, 6.889597, 0.034448 );
    M_static_table_v18_wide.emplace_back( 7.00, 6.993983, 0.069938 );
    M_static_table_v18_wide.emplace_back( 7.10, 7.099417, 0.035497 );
    M_static_table_v18_wide.emplace_back( 7.20, 7.206981, 0.072068 );
    M_static_table_v18_wide.emplace_back( 7.30, 7.315626, 0.036578 );
    M_static_table_v18_wide.emplace_back( 7.40, 7.389149, 0.036945 );
    M_static_table_v18_wide.emplace_back( 7.50, 7.501103, 0.075009 );
    M_static_table_v18_wide.emplace_back( 7.60, 7.614182, 0.038071 );
    M_static_table_v18_wide.emplace_back( 7.70, 7.690706, 0.038453 );
    M_static_table_v18_wide.emplace_back( 7.80, 7.807229, 0.078070 );
    M_static_table_v18_wide.emplace_back( 7.90, 7.924923, 0.039625 );
    M_static_table_v18_wide.emplace_back( 8.00, 8.004570, 0.040022 );
    M_static_table_v18_wide.emplace_back( 8.10, 8.085017, 0.040424 );
    M_static_table_v18_wide.emplace_back( 8.20, 8.207514, 0.082072 );
    M_static_table_v18_wide.emplace_back( 8.30, 8.331242, 0.041656 );
    M_static_table_v18_wide.emplace_back( 8.40, 8.414973, 0.042074 );
    M_static_table_v18_wide.emplace_back( 8.50, 8.499545, 0.042497 );
    M_static_table_v18_wide.emplace_back( 8.60, 8.584967, 0.042924 );
    M_static_table_v18_wide.emplace_back( 8.70, 8.671247, 0.043355 );
    M_static_table_v18_wide.emplace_back( 8.80, 8.802626, 0.088023 );
    M_static_table_v18_wide.emplace_back( 8.90, 8.935326, 0.044676 );
    M_static_table_v18_wide.emplace_back( 9.00, 9.025127, 0.045125 );
    M_static_table_v18_wide.emplace_back( 9.10, 9.115831, 0.045579 );
    M_static_table_v18_wide.emplace_back( 9.20, 9.207446, 0.046036 );
    M_static_table_v18_wide.emplace_back( 9.30, 9.299982, 0.046499 );
    M_static_table_v18_wide.emplace_back( 9.40, 9.393449, 0.046967 );
    M_static_table_v18_wide.emplace_back( 9.50, 9.487855, 0.047439 );
    M_static_table_v18_wide.emplace_back( 9.60, 9.583209, 0.047915 );
    M_static_table_v18_wide.emplace_back( 9.70, 9.679522, 0.048397 );
    M_static_table_v18_wide.emplace_back( 9.80, 9.776803, 0.048883 );
    M_static_table_v18_wide.emplace_back( 9.90, 9.875062, 0.049375 );
    M_static_table_v18_wide.emplace_back( 10.00, 9.974308, 0.049871 );
    M_static_table_v18_wide.emplace_back( 10.10, 10.074551, 0.050372 );
    M_static_table_v18_wide.emplace_back( 10.20, 10.175802, 0.050879 );
    M_static_table_v18_wide.emplace_back( 10.30, 10.278070, 0.051389 );
    M_static_table_v18_wide.emplace_back( 10.40, 10.381366, 0.051906 );
    M_static_table_v18_wide.emplace_back( 10.50, 10.485701, 0.052428 );
    M_static_table_v18_wide.emplace_back( 10.60, 10.591084, 0.052955 );
    M_static_table_v18_wide.emplace_back( 10.70, 10.697526, 0.053487 );
    M_static_table_v18_wide.emplace_back( 10.80, 10.805038, 0.054024 );
    M_static_table_v18_wide.emplace_back( 10.90, 10.913631, 0.054568 );
    M_static_table_v18_wide.emplace_back( 11.00, 11.023315, 0.055116 );
    M_static_table_v18_wide.emplace_back( 11.10, 11.134101, 0.055670 );
    M_static_table_v18_wide.emplace_back( 11.20, 11.246000, 0.056229 );
    M_static_table_v18_wide.emplace_back( 11.40, 11.359024, 0.056794 );
    M_static_table_v18_wide.emplace_back( 11.50, 11.473184, 0.057365 );
    M_static_table_v18_wide.emplace_back( 11.60, 11.588492, 0.057942 );
    M_static_table_v18_wide.emplace_back( 11.70, 11.704958, 0.058524 );
    M_static_table_v18_wide.emplace_back( 11.80, 11.822595, 0.059112 );
    M_static_table_v18_wide.emplace_back( 11.90, 11.941414, 0.059706 );
    M_static_table_v18_wide.emplace_back( 12.10, 12.061427, 0.060306 );
    M_static_table_v18_wide.emplace_back( 12.20, 12.182646, 0.060912 );
    M_static_table_v18_wide.emplace_back( 12.30, 12.305084, 0.061525 );
    M_static_table_v18_wide.emplace_back( 12.40, 12.428752, 0.062143 );
    M_static_table_v18_wide.emplace_back( 12.60, 12.553663, 0.062767 );
    M_static_table_v18_wide.emplace_back( 12.70, 12.679830, 0.063399 );
    M_static_table_v18_wide.emplace_back( 12.80, 12.807264, 0.064035 );
    M_static_table_v18_wide.emplace_back( 12.90, 12.935979, 0.064679 );
    M_static_table_v18_wide.emplace_back( 13.10, 13.065988, 0.065329 );
    M_static_table_v18_wide.emplace_back( 13.20, 13.197304, 0.065986 );
    M_static_table_v18_wide.emplace_back( 13.30, 13.329939, 0.066649 );
    M_static_table_v18_wide.emplace_back( 13.50, 13.463907, 0.067319 );
    M_static_table_v18_wide.emplace_back( 13.60, 13.599221, 0.067995 );
    M_static_table_v18_wide.emplace_back( 13.70, 13.735896, 0.068679 );
    M_static_table_v18_wide.emplace_back( 13.90, 13.873944, 0.069369 );
    M_static_table_v18_wide.emplace_back( 14.00, 14.013379, 0.070066 );
    M_static_table_v18_wide.emplace_back( 14.20, 14.154216, 0.070770 );
    M_static_table_v18_wide.emplace_back( 14.30, 14.296468, 0.071481 );
    M_static_table_v18_wide.emplace_back( 14.40, 14.440150, 0.072200 );
    M_static_table_v18_wide.emplace_back( 14.60, 14.585276, 0.072926 );
    M_static_table_v18_wide.emplace_back( 14.70, 14.731860, 0.073658 );
    M_static_table_v18_wide.emplace_back( 14.90, 14.879918, 0.074399 );
    M_static_table_v18_wide.emplace_back( 15.00, 15.029464, 0.075147 );
    M_static_table_v18_wide.emplace_back( 15.20, 15.180512, 0.075901 );
    M_static_table_v18_wide.emplace_back( 15.30, 15.333079, 0.076665 );
    M_static_table_v18_wide.emplace_back( 15.50, 15.487179, 0.077435 );
    M_static_table_v18_wide.emplace_back( 15.60, 15.642827, 0.078213 );
    M_static_table_v18_wide.emplace_back( 15.80, 15.800040, 0.078999 );
    M_static_table_v18_wide.emplace_back( 16.00, 15.958834, 0.079794 );
    M_static_table_v18_wide.emplace_back( 16.10, 16.119223, 0.080595 );
    M_static_table_v18_wide.emplace_back( 16.30, 16.281223, 0.081406 );
    M_static_table_v18_wide.emplace_back( 16.40, 16.444852, 0.082224 );
    M_static_table_v18_wide.emplace_back( 16.60, 16.610126, 0.083050 );
    M_static_table_v18_wide.emplace_back( 16.80, 16.777060, 0.083885 );
    M_static_table_v18_wide.emplace_back( 16.90, 16.945673, 0.084728 );
    M_static_table_v18_wide.emplace_back( 17.10, 17.115980, 0.085579 );
    M_static_table_v18_wide.emplace_back( 17.30, 17.287998, 0.086440 );
    M_static_table_v18_wide.emplace_back( 17.50, 17.461746, 0.087308 );
    M_static_table_v18_wide.emplace_back( 17.60, 17.637239, 0.088186 );
    M_static_table_v18_wide.emplace_back( 17.80, 17.814496, 0.089072 );
    M_static_table_v18_wide.emplace_back( 18.00, 17.993535, 0.089967 );
    M_static_table_v18_wide.emplace_back( 18.20, 18.174373, 0.090871 );
    M_static_table_v18_wide.emplace_back( 18.40, 18.357028, 0.091785 );
    M_static_table_v18_wide.emplace_back( 18.50, 18.541520, 0.092707 );
    M_static_table_v18_wide.emplace_back( 18.70, 18.727865, 0.093639 );
    M_static_table_v18_wide.emplace_back( 18.90, 18.916083, 0.094580 );
    M_static_table_v18_wide.emplace_back( 19.10, 19.106193, 0.095530 );
    M_static_table_v18_wide.emplace_back( 19.30, 19.298213, 0.096491 );
    M_static_table_v18_wide.emplace_back( 19.50, 19.492164, 0.097460 );
    M_static_table_v18_wide.emplace_back( 19.70, 19.688063, 0.098440 );
    M_static_table_v18_wide.emplace_back( 19.90, 19.885931, 0.099429 );
    M_static_table_v18_wide.emplace_back( 20.10, 20.085788, 0.100429 );
    M_static_table_v18_wide.emplace_back( 20.30, 20.287654, 0.101437 );
    M_static_table_v18_wide.emplace_back( 20.50, 20.491548, 0.102457 );
    M_static_table_v18_wide.emplace_back( 20.70, 20.697491, 0.103487 );
    M_static_table_v18_wide.emplace_back( 20.90, 20.905505, 0.104527 );
    M_static_table_v18_wide.emplace_back( 21.10, 21.115609, 0.105577 );
    M_static_table_v18_wide.emplace_back( 21.30, 21.327824, 0.106638 );
    M_static_table_v18_wide.emplace_back( 21.50, 21.542172, 0.107710 );
    M_static_table_v18_wide.emplace_back( 21.80, 21.758675, 0.108793 );
    M_static_table_v18_wide.emplace_back( 22.00, 21.977353, 0.109886 );
    M_static_table_v18_wide.emplace_back( 22.20, 22.198229, 0.110990 );
    M_static_table_v18_wide.emplace_back( 22.40, 22.421325, 0.112106 );
    M_static_table_v18_wide.emplace_back( 22.60, 22.646664, 0.113233 );
    M_static_table_v18_wide.emplace_back( 22.90, 22.874266, 0.114370 );
    M_static_table_v18_wide.emplace_back( 23.10, 23.104156, 0.115520 );
    M_static_table_v18_wide.emplace_back( 23.30, 23.336357, 0.116681 );
    M_static_table_v18_wide.emplace_back( 23.60, 23.570892, 0.117854 );
    M_static_table_v18_wide.emplace_back( 23.80, 23.807783, 0.119038 );
    M_static_table_v18_wide.emplace_back( 24.00, 24.047055, 0.120235 );
    M_static_table_v18_wide.emplace_back( 24.30, 24.288732, 0.121443 );
    M_static_table_v18_wide.emplace_back( 24.50, 24.532838, 0.122664 );
    M_static_table_v18_wide.emplace_back( 24.80, 24.779397, 0.123896 );
    M_static_table_v18_wide.emplace_back( 25.00, 25.028434, 0.125142 );
    M_static_table_v18_wide.emplace_back( 25.30, 25.279974, 0.126399 );
    M_static_table_v18_wide.emplace_back( 25.50, 25.534042, 0.127670 );
    M_static_table_v18_wide.emplace_back( 25.80, 25.790663, 0.128952 );
    M_static_table_v18_wide.emplace_back( 26.00, 26.049863, 0.130248 );
    M_static_table_v18_wide.emplace_back( 26.30, 26.311669, 0.131558 );
    M_static_table_v18_wide.emplace_back( 26.60, 26.576106, 0.132880 );
    M_static_table_v18_wide.emplace_back( 26.80, 26.843200, 0.134215 );
    M_static_table_v18_wide.emplace_back( 27.10, 27.112979, 0.135564 );
    M_static_table_v18_wide.emplace_back( 27.40, 27.385468, 0.136926 );
    M_static_table_v18_wide.emplace_back( 27.70, 27.660697, 0.138303 );
    M_static_table_v18_wide.emplace_back( 27.90, 27.938692, 0.139693 );
    M_static_table_v18_wide.emplace_back( 28.20, 28.219480, 0.141096 );
    M_static_table_v18_wide.emplace_back( 28.50, 28.503091, 0.142515 );
    M_static_table_v18_wide.emplace_back( 28.80, 28.789552, 0.143947 );
    M_static_table_v18_wide.emplace_back( 29.10, 29.078891, 0.145393 );
    M_static_table_v18_wide.emplace_back( 29.40, 29.371139, 0.146855 );
    M_static_table_v18_wide.emplace_back( 29.70, 29.666324, 0.148331 );
    M_static_table_v18_wide.emplace_back( 30.00, 29.964475, 0.149821 );
    M_static_table_v18_wide.emplace_back( 30.30, 30.265623, 0.151327 );
    M_static_table_v18_wide.emplace_back( 30.60, 30.569798, 0.152848 );
    M_static_table_v18_wide.emplace_back( 30.90, 30.877029, 0.154384 );
    M_static_table_v18_wide.emplace_back( 31.20, 31.187349, 0.155936 );
    M_static_table_v18_wide.emplace_back( 31.50, 31.500787, 0.157503 );
    M_static_table_v18_wide.emplace_back( 31.80, 31.817375, 0.159086 );
    M_static_table_v18_wide.emplace_back( 32.10, 32.137145, 0.160684 );
    M_static_table_v18_wide.emplace_back( 32.50, 32.460129, 0.162299 );
    M_static_table_v18_wide.emplace_back( 32.80, 32.786358, 0.163930 );
    M_static_table_v18_wide.emplace_back( 33.10, 33.115866, 0.165578 );
    M_static_table_v18_wide.emplace_back( 33.40, 33.448686, 0.167242 );
    M_static_table_v18_wide.emplace_back( 33.80, 33.784851, 0.168923 );
    M_static_table_v18_wide.emplace_back( 34.10, 34.124395, 0.170620 );
    M_static_table_v18_wide.emplace_back( 34.50, 34.467351, 0.172335 );
    M_static_table_v18_wide.emplace_back( 34.80, 34.813753, 0.174067 );
    M_static_table_v18_wide.emplace_back( 35.20, 35.163637, 0.175817 );
    M_static_table_v18_wide.emplace_back( 35.50, 35.517038, 0.177583 );
    M_static_table_v18_wide.emplace_back( 35.90, 35.873990, 0.179368 );
    M_static_table_v18_wide.emplace_back( 36.20, 36.234529, 0.181171 );
    M_static_table_v18_wide.emplace_back( 36.60, 36.598692, 0.182992 );
    M_static_table_v18_wide.emplace_back( 37.00, 36.966515, 0.184831 );
    M_static_table_v18_wide.emplace_back( 37.30, 37.338035, 0.186689 );
    M_static_table_v18_wide.emplace_back( 37.70, 37.713288, 0.188564 );
    M_static_table_v18_wide.emplace_back( 38.10, 38.092313, 0.190460 );
    M_static_table_v18_wide.emplace_back( 38.50, 38.475147, 0.192374 );
    M_static_table_v18_wide.emplace_back( 38.90, 38.861829, 0.194307 );
    M_static_table_v18_wide.emplace_back( 39.30, 39.252397, 0.196260 );
    M_static_table_v18_wide.emplace_back( 39.60, 39.646890, 0.198233 );
    M_static_table_v18_wide.emplace_back( 40.00, 40.045348, 0.200225 );
    M_static_table_v18_wide.emplace_back( 40.40, 40.447810, 0.202237 );
    M_static_table_v18_wide.emplace_back( 40.90, 40.854318, 0.204270 );
    M_static_table_v18_wide.emplace_back( 41.30, 41.264910, 0.206322 );
    M_static_table_v18_wide.emplace_back( 41.70, 41.679629, 0.208396 );
    M_static_table_v18_wide.emplace_back( 42.10, 42.098517, 0.210491 );
    M_static_table_v18_wide.emplace_back( 42.50, 42.521614, 0.212606 );
    M_static_table_v18_wide.emplace_back( 42.90, 42.948963, 0.214743 );
    M_static_table_v18_wide.emplace_back( 43.40, 43.380607, 0.216901 );
    M_static_table_v18_wide.emplace_back( 43.80, 43.816590, 0.219081 );
    M_static_table_v18_wide.emplace_back( 44.30, 44.256954, 0.221283 );
    M_static_table_v18_wide.emplace_back( 44.70, 44.701744, 0.223507 );
    M_static_table_v18_wide.emplace_back( 45.20, 45.151004, 0.225753 );
    M_static_table_v18_wide.emplace_back( 45.60, 45.604779, 0.228022 );
    M_static_table_v18_wide.emplace_back( 46.10, 46.063114, 0.230313 );
    M_static_table_v18_wide.emplace_back( 46.50, 46.526056, 0.232628 );
    M_static_table_v18_wide.emplace_back( 47.00, 46.993651, 0.234966 );
    M_static_table_v18_wide.emplace_back( 47.50, 47.465945, 0.237328 );
    M_static_table_v18_wide.emplace_back( 47.90, 47.942986, 0.239713 );
    M_static_table_v18_wide.emplace_back( 48.40, 48.424821, 0.242122 );
    M_static_table_v18_wide.emplace_back( 48.90, 48.911498, 0.244555 );
    M_static_table_v18_wide.emplace_back( 49.40, 49.403067, 0.247013 );
    M_static_table_v18_wide.emplace_back( 49.90, 49.899576, 0.249496 );
    M_static_table_v18_wide.emplace_back( 50.40, 50.401075, 0.252003 );
    M_static_table_v18_wide.emplace_back( 50.90, 50.907614, 0.254535 );
    M_static_table_v18_wide.emplace_back( 51.40, 51.419244, 0.257094 );
    M_static_table_v18_wide.emplace_back( 51.90, 51.936016, 0.259677 );
    M_static_table_v18_wide.emplace_back( 52.50, 52.457982, 0.262288 );
    M_static_table_v18_wide.emplace_back( 53.00, 52.985193, 0.264923 );
    M_static_table_v18_wide.emplace_back( 53.50, 53.517703, 0.267586 );
    M_static_table_v18_wide.emplace_back( 54.10, 54.055565, 0.270275 );
    M_static_table_v18_wide.emplace_back( 54.60, 54.598833, 0.272992 );
    M_static_table_v18_wide.emplace_back( 55.10, 55.147560, 0.275735 );
    M_static_table_v18_wide.emplace_back( 55.70, 55.701802, 0.278506 );
    M_static_table_v18_wide.emplace_back( 56.30, 56.261615, 0.281306 );
    M_static_table_v18_wide.emplace_back( 56.80, 56.827054, 0.284133 );
    M_static_table_v18_wide.emplace_back( 57.40, 57.398175, 0.286988 );
    M_static_table_v18_wide.emplace_back( 58.00, 57.975036, 0.289872 );
    M_static_table_v18_wide.emplace_back( 58.60, 58.557695, 0.292786 );
    M_static_table_v18_wide.emplace_back( 59.10, 59.146209, 0.295728 );
    M_static_table_v18_wide.emplace_back( 59.70, 59.740639, 0.298701 );
    M_static_table_v18_wide.emplace_back( 60.30, 60.341042, 0.301702 );
    M_static_table_v18_wide.emplace_back( 60.90, 60.947480, 0.304735 );
    M_static_table_v18_wide.emplace_back( 61.60, 61.560012, 0.307797 );
    M_static_table_v18_wide.emplace_back( 62.20, 62.178701, 0.310891 );
    M_static_table_v18_wide.emplace_back( 62.80, 62.803607, 0.314015 );
    M_static_table_v18_wide.emplace_back( 63.40, 63.434793, 0.317171 );
    M_static_table_v18_wide.emplace_back( 64.10, 64.072324, 0.320359 );
    M_static_table_v18_wide.emplace_back( 64.70, 64.716261, 0.323578 );
    M_static_table_v18_wide.emplace_back( 65.40, 65.366670, 0.326830 );
    M_static_table_v18_wide.emplace_back( 66.00, 66.023616, 0.330115 );
    M_static_table_v18_wide.emplace_back( 66.70, 66.687165, 0.333433 );
    M_static_table_v18_wide.emplace_back( 67.40, 67.357382, 0.336784 );
    M_static_table_v18_wide.emplace_back( 68.00, 68.034335, 0.340169 );
    M_static_table_v18_wide.emplace_back( 68.70, 68.718091, 0.343587 );
    M_static_table_v18_wide.emplace_back( 69.40, 69.408720, 0.347041 );
    M_static_table_v18_wide.emplace_back( 70.10, 70.106289, 0.350528 );
    M_static_table_v18_wide.emplace_back( 70.80, 70.810869, 0.354051 );
    M_static_table_v18_wide.emplace_back( 71.50, 71.522530, 0.357609 );
    M_static_table_v18_wide.emplace_back( 72.20, 72.241343, 0.361203 );
    M_static_table_v18_wide.emplace_back( 73.00, 72.967381, 0.364834 );
    M_static_table_v18_wide.emplace_back( 73.70, 73.700715, 0.368500 );
    M_static_table_v18_wide.emplace_back( 74.40, 74.441420, 0.372204 );
    M_static_table_v18_wide.emplace_back( 75.20, 75.189568, 0.375944 );
    M_static_table_v18_wide.emplace_back( 75.90, 75.945236, 0.379723 );
    M_static_table_v18_wide.emplace_back( 76.70, 76.708498, 0.383539 );
    M_static_table_v18_wide.emplace_back( 77.50, 77.479432, 0.387394 );
    M_static_table_v18_wide.emplace_back( 78.30, 78.258113, 0.391287 );
    M_static_table_v18_wide.emplace_back( 79.00, 79.044620, 0.395219 );
    M_static_table_v18_wide.emplace_back( 79.80, 79.839032, 0.399192 );
    M_static_table_v18_wide.emplace_back( 80.60, 80.641427, 0.403203 );
    M_static_table_v18_wide.emplace_back( 81.50, 81.451887, 0.407256 );
    M_static_table_v18_wide.emplace_back( 82.30, 82.270492, 0.411349 );
    M_static_table_v18_wide.emplace_back( 83.10, 83.097325, 0.415483 );
    M_static_table_v18_wide.emplace_back( 83.90, 83.932467, 0.419659 );
    M_static_table_v18_wide.emplace_back( 84.80, 84.776002, 0.423876 );
    M_static_table_v18_wide.emplace_back( 85.60, 85.628014, 0.428136 );
    M_static_table_v18_wide.emplace_back( 86.50, 86.488590, 0.432439 );
    M_static_table_v18_wide.emplace_back( 87.40, 87.357815, 0.436785 );
    M_static_table_v18_wide.emplace_back( 88.20, 88.235776, 0.441175 );
    M_static_table_v18_wide.emplace_back( 89.10, 89.122560, 0.445609 );
    M_static_table_v18_wide.emplace_back( 90.00, 90.018257, 0.450087 );
    M_static_table_v18_wide.emplace_back( 90.90, 90.922956, 0.454611 );
    M_static_table_v18_wide.emplace_back( 91.80, 91.836746, 0.459179 );
    M_static_table_v18_wide.emplace_back( 92.80, 92.759721, 0.463795 );
    M_static_table_v18_wide.emplace_back( 93.70, 93.691972, 0.468456 );
    M_static_table_v18_wide.emplace_back( 94.60, 94.633592, 0.473164 );
    M_static_table_v18_wide.emplace_back( 95.60, 95.584675, 0.477919 );
    M_static_table_v18_wide.emplace_back( 96.50, 96.545317, 0.482722 );
    M_static_table_v18_wide.emplace_back( 97.50, 97.515614, 0.487574 );
    M_static_table_v18_wide.emplace_back( 98.50, 98.495662, 0.492474 );
    M_static_table_v18_wide.emplace_back( 99.50, 99.485560, 0.497424 );
    M_static_table_v18_wide.emplace_back( 100.50, 100.485406, 0.502422 );
    M_static_table_v18_wide.emplace_back( 101.50, 101.495301, 0.507472 );
    M_static_table_v18_wide.emplace_back( 102.50, 102.515346, 0.512572 );
    M_static_table_v18_wide.emplace_back( 103.50, 103.545642, 0.517723 );
    M_static_table_v18_wide.emplace_back( 104.60, 104.586293, 0.522927 );
    M_static_table_v18_wide.emplace_back( 105.60, 105.637403, 0.528182 );
    M_static_table_v18_wide.emplace_back( 106.70, 106.699077, 0.533491 );
    M_static_table_v18_wide.emplace_back( 107.80, 107.771420, 0.538852 );
    M_static_table_v18_wide.emplace_back( 108.90, 108.854541, 0.544268 );
    M_static_table_v18_wide.emplace_back( 109.90, 109.948547, 0.549738 );
    M_static_table_v18_wide.emplace_back( 111.10, 111.053549, 0.555263 );
    M_static_table_v18_wide.emplace_back( 112.20, 112.169655, 0.560843 );
    M_static_table_v18_wide.emplace_back( 113.30, 113.296979, 0.566480 );
    M_static_table_v18_wide.emplace_back( 114.40, 114.435632, 0.572173 );
    M_static_table_v18_wide.emplace_back( 115.60, 115.585730, 0.577924 );
    M_static_table_v18_wide.emplace_back( 116.70, 116.747386, 0.583732 );
    M_static_table_v18_wide.emplace_back( 117.90, 117.920716, 0.589598 );
    M_static_table_v18_wide.emplace_back( 119.10, 119.105839, 0.595524 );
    M_static_table_v18_wide.emplace_back( 120.30, 120.302873, 0.601509 );
    M_static_table_v18_wide.emplace_back( 121.50, 121.511937, 0.607555 );
    M_static_table_v18_wide.emplace_back( 122.70, 122.733152, 0.613660 );
    M_static_table_v18_wide.emplace_back( 124.00, 123.966641, 0.619828 );
    M_static_table_v18_wide.emplace_back( 125.20, 125.212526, 0.626057 );
    M_static_table_v18_wide.emplace_back( 126.50, 126.470933, 0.632349 );
    M_static_table_v18_wide.emplace_back( 127.70, 127.741987, 0.638704 );
    M_static_table_v18_wide.emplace_back( 129.00, 129.025815, 0.645123 );
    M_static_table_v18_wide.emplace_back( 130.30, 130.322546, 0.651607 );
    M_static_table_v18_wide.emplace_back( 131.60, 131.632310, 0.658156 );
    M_static_table_v18_wide.emplace_back( 133.00, 132.955236, 0.664770 );
    M_static_table_v18_wide.emplace_back( 134.30, 134.291459, 0.671452 );
    M_static_table_v18_wide.emplace_back( 135.60, 135.641111, 0.678200 );
    M_static_table_v18_wide.emplace_back( 137.00, 137.004326, 0.685015 );
    M_static_table_v18_wide.emplace_back( 138.40, 138.381242, 0.691900 );
    M_static_table_v18_wide.emplace_back( 139.80, 139.771997, 0.698854 );
    M_static_table_v18_wide.emplace_back( 141.20, 141.176729, 0.705877 );
    M_static_table_v18_wide.emplace_back( 142.60, 142.595579, 0.712972 );
    M_static_table_v18_wide.emplace_back( 144.00, 144.028688, 0.720137 );
    M_static_table_v18_wide.emplace_back( 145.50, 145.476201, 0.727375 );
    M_static_table_v18_wide.emplace_back( 146.90, 146.938261, 0.734685 );
    M_static_table_v18_wide.emplace_back( 148.40, 148.415015, 0.742069 );
    M_static_table_v18_wide.emplace_back( 149.90, 149.906611, 0.749527 );
    M_static_table_v18_wide.emplace_back( 151.40, 151.413197, 0.757059 );

    //
    M_movable_table.emplace_back( 0.00, 0.026170, 0.026170 );
    M_movable_table.emplace_back( 0.10, 0.104789, 0.052450 );
    M_movable_table.emplace_back( 0.20, 0.208239, 0.051002 );
    M_movable_table.emplace_back( 0.30, 0.304589, 0.045349 );
    M_movable_table.emplace_back( 0.40, 0.411152, 0.061215 );
    M_movable_table.emplace_back( 0.50, 0.524658, 0.052292 );
    M_movable_table.emplace_back( 0.60, 0.607289, 0.030340 );
    M_movable_table.emplace_back( 0.70, 0.708214, 0.070587 );
    M_movable_table.emplace_back( 0.80, 0.819754, 0.040954 );
    M_movable_table.emplace_back( 0.90, 0.905969, 0.045262 );
    M_movable_table.emplace_back( 1.00, 1.001251, 0.050021 );
    M_movable_table.emplace_back( 1.10, 1.106553, 0.055282 );
    M_movable_table.emplace_back( 1.20, 1.222930, 0.061096 );
    M_movable_table.emplace_back( 1.30, 1.351546, 0.067521 );
    M_movable_table.emplace_back( 1.50, 1.493690, 0.074623 );
    M_movable_table.emplace_back( 1.60, 1.650783, 0.082471 );
    M_movable_table.emplace_back( 1.80, 1.824397, 0.091144 );
    M_movable_table.emplace_back( 2.00, 2.016270, 0.100731 );
    M_movable_table.emplace_back( 2.20, 2.228323, 0.111324 );
    M_movable_table.emplace_back( 2.50, 2.462678, 0.123032 );
    M_movable_table.emplace_back( 2.70, 2.721681, 0.135972 );
    M_movable_table.emplace_back( 3.00, 3.007922, 0.150271 );
    M_movable_table.emplace_back( 3.30, 3.324268, 0.166076 );
    M_movable_table.emplace_back( 3.70, 3.673884, 0.183542 );
    M_movable_table.emplace_back( 4.10, 4.060270, 0.202845 );
    M_movable_table.emplace_back( 4.50, 4.487293, 0.224179 );
    M_movable_table.emplace_back( 5.00, 4.959225, 0.247755 );
    M_movable_table.emplace_back( 5.50, 5.480791, 0.273812 );
    M_movable_table.emplace_back( 6.00, 6.057211, 0.302609 );
    M_movable_table.emplace_back( 6.70, 6.694254, 0.334435 );
    M_movable_table.emplace_back( 7.40, 7.398295, 0.369608 );
    M_movable_table.emplace_back( 8.20, 8.176380, 0.408479 );
    M_movable_table.emplace_back( 9.00, 9.036297, 0.451439 );
    M_movable_table.emplace_back( 10.00, 9.986652, 0.498917 );
    M_movable_table.emplace_back( 11.00, 11.036958, 0.551389 );
    M_movable_table.emplace_back( 12.20, 12.197725, 0.609379 );
    M_movable_table.emplace_back( 13.50, 13.480571, 0.673468 );
    M_movable_table.emplace_back( 14.90, 14.898335, 0.744297 );
    M_movable_table.emplace_back( 16.40, 16.465206, 0.822576 );
    M_movable_table.emplace_back( 18.20, 18.196867, 0.909087 );
    M_movable_table.emplace_back( 20.10, 20.110649, 1.004696 );
    M_movable_table.emplace_back( 22.20, 22.225705, 1.110361 );
    M_movable_table.emplace_back( 24.50, 24.563202, 1.227138 );
    M_movable_table.emplace_back( 27.10, 27.146537, 1.356198 );
    M_movable_table.emplace_back( 30.00, 30.001563, 1.498830 );
    M_movable_table.emplace_back( 33.10, 33.156855, 1.656463 );
    M_movable_table.emplace_back( 36.60, 36.643992, 1.830675 );
    M_movable_table.emplace_back( 40.40, 40.497874, 2.023208 );
    M_movable_table.emplace_back( 44.70, 44.757073, 2.235991 );
    M_movable_table.emplace_back( 49.40, 49.464215, 2.471152 );
    M_movable_table.emplace_back( 54.60, 54.666412, 2.731046 );
    M_movable_table.emplace_back( 60.30, 60.415729, 3.018272 );
    M_movable_table.emplace_back( 66.70, 66.769706, 3.335706 );
    M_movable_table.emplace_back( 73.70, 73.791938, 3.686526 );
    M_movable_table.emplace_back( 81.50, 81.552704, 4.074241 );
    M_movable_table.emplace_back( 90.00, 90.129676, 4.502732 );
    M_movable_table.emplace_back( 99.50, 99.608697, 4.976289 );
    M_movable_table.emplace_back( 109.90, 110.084635, 5.499650 );
    M_movable_table.emplace_back( 121.50, 121.662337, 6.078053 );
    M_movable_table.emplace_back( 134.30, 134.457677, 6.717287 );
    M_movable_table.emplace_back( 148.40, 148.598714, 7.423750 );

    //
    M_movable_table_v18_narrow.emplace_back( 0.00, 0.025524, 0.025524 );
    M_movable_table_v18_narrow.emplace_back( 0.10, 0.102202, 0.051154 );
    M_movable_table_v18_narrow.emplace_back( 0.20, 0.203098, 0.049743 );
    M_movable_table_v18_narrow.emplace_back( 0.30, 0.305818, 0.052978 );
    M_movable_table_v18_narrow.emplace_back( 0.40, 0.409750, 0.050953 );
    M_movable_table_v18_narrow.emplace_back( 0.50, 0.511704, 0.051001 );
    M_movable_table_v18_narrow.emplace_back( 0.60, 0.608237, 0.045533 );
    M_movable_table_v18_narrow.emplace_back( 0.70, 0.706671, 0.052902 );
    M_movable_table_v18_narrow.emplace_back( 0.80, 0.799516, 0.039943 );
    M_movable_table_v18_narrow.emplace_back( 0.90, 0.883601, 0.044143 );
    M_movable_table_v18_narrow.emplace_back( 1.00, 0.976530, 0.048786 );
    M_movable_table_v18_narrow.emplace_back( 1.10, 1.079233, 0.053916 );
    M_movable_table_v18_narrow.emplace_back( 1.20, 1.192736, 0.059587 );
    M_movable_table_v18_narrow.emplace_back( 1.30, 1.318177, 0.065854 );
    M_movable_table_v18_narrow.emplace_back( 1.40, 1.419511, 0.035480 );
    M_movable_table_v18_narrow.emplace_back( 1.50, 1.492291, 0.037299 );
    M_movable_table_v18_narrow.emplace_back( 1.60, 1.610025, 0.080434 );
    M_movable_table_v18_narrow.emplace_back( 1.70, 1.733795, 0.043336 );
    M_movable_table_v18_narrow.emplace_back( 1.80, 1.822688, 0.045557 );
    M_movable_table_v18_narrow.emplace_back( 1.90, 1.916139, 0.047893 );
    M_movable_table_v18_narrow.emplace_back( 2.00, 2.014382, 0.050350 );
    M_movable_table_v18_narrow.emplace_back( 2.10, 2.117662, 0.052931 );
    M_movable_table_v18_narrow.emplace_back( 2.20, 2.226237, 0.055644 );
    M_movable_table_v18_narrow.emplace_back( 2.30, 2.340378, 0.058498 );
    M_movable_table_v18_narrow.emplace_back( 2.50, 2.460372, 0.061497 );
    M_movable_table_v18_narrow.emplace_back( 2.60, 2.586519, 0.064650 );
    M_movable_table_v18_narrow.emplace_back( 2.70, 2.719132, 0.067964 );
    M_movable_table_v18_narrow.emplace_back( 2.90, 2.858545, 0.071449 );
    M_movable_table_v18_narrow.emplace_back( 3.00, 3.005105, 0.075112 );
    M_movable_table_v18_narrow.emplace_back( 3.20, 3.159180, 0.078963 );
    M_movable_table_v18_narrow.emplace_back( 3.30, 3.321155, 0.083012 );
    M_movable_table_v18_narrow.emplace_back( 3.50, 3.491435, 0.087268 );
    M_movable_table_v18_narrow.emplace_back( 3.70, 3.670444, 0.091742 );
    M_movable_table_v18_narrow.emplace_back( 3.90, 3.858632, 0.096446 );
    M_movable_table_v18_narrow.emplace_back( 4.10, 4.056468, 0.101391 );
    M_movable_table_v18_narrow.emplace_back( 4.30, 4.264447, 0.106589 );
    M_movable_table_v18_narrow.emplace_back( 4.50, 4.483090, 0.112054 );
    M_movable_table_v18_narrow.emplace_back( 4.70, 4.712943, 0.117799 );
    M_movable_table_v18_narrow.emplace_back( 5.00, 4.954581, 0.123839 );
    M_movable_table_v18_narrow.emplace_back( 5.20, 5.208608, 0.130188 );
    M_movable_table_v18_narrow.emplace_back( 5.50, 5.475659, 0.136863 );
    M_movable_table_v18_narrow.emplace_back( 5.80, 5.756402, 0.143880 );
    M_movable_table_v18_narrow.emplace_back( 6.00, 6.051539, 0.151257 );
    M_movable_table_v18_narrow.emplace_back( 6.40, 6.361808, 0.159012 );
    M_movable_table_v18_narrow.emplace_back( 6.70, 6.687985, 0.167165 );
    M_movable_table_v18_narrow.emplace_back( 7.00, 7.030885, 0.175736 );
    M_movable_table_v18_narrow.emplace_back( 7.40, 7.391366, 0.184746 );
    M_movable_table_v18_narrow.emplace_back( 7.80, 7.770329, 0.194218 );
    M_movable_table_v18_narrow.emplace_back( 8.20, 8.168723, 0.204175 );
    M_movable_table_v18_narrow.emplace_back( 8.60, 8.587542, 0.214644 );
    M_movable_table_v18_narrow.emplace_back( 9.00, 9.027834, 0.225648 );
    M_movable_table_v18_narrow.emplace_back( 9.50, 9.490701, 0.237218 );
    M_movable_table_v18_narrow.emplace_back( 10.00, 9.977300, 0.249380 );
    M_movable_table_v18_narrow.emplace_back( 10.50, 10.488847, 0.262166 );
    M_movable_table_v18_narrow.emplace_back( 11.00, 11.026622, 0.275608 );
    M_movable_table_v18_narrow.emplace_back( 11.60, 11.591969, 0.289739 );
    M_movable_table_v18_narrow.emplace_back( 12.20, 12.186302, 0.304594 );
    M_movable_table_v18_narrow.emplace_back( 12.80, 12.811107, 0.320211 );
    M_movable_table_v18_narrow.emplace_back( 13.50, 13.467946, 0.336628 );
    M_movable_table_v18_narrow.emplace_back( 14.20, 14.158462, 0.353887 );
    M_movable_table_v18_narrow.emplace_back( 14.90, 14.884382, 0.372032 );
    M_movable_table_v18_narrow.emplace_back( 15.60, 15.647521, 0.391107 );
    M_movable_table_v18_narrow.emplace_back( 16.40, 16.449786, 0.411159 );
    M_movable_table_v18_narrow.emplace_back( 17.30, 17.293185, 0.432240 );
    M_movable_table_v18_narrow.emplace_back( 18.20, 18.179826, 0.454401 );
    M_movable_table_v18_narrow.emplace_back( 19.10, 19.111925, 0.477699 );
    M_movable_table_v18_narrow.emplace_back( 20.10, 20.091814, 0.502191 );
    M_movable_table_v18_narrow.emplace_back( 21.10, 21.121944, 0.527939 );
    M_movable_table_v18_narrow.emplace_back( 22.20, 22.204889, 0.555007 );
    M_movable_table_v18_narrow.emplace_back( 23.30, 23.343358, 0.583462 );
    M_movable_table_v18_narrow.emplace_back( 24.50, 24.540198, 0.613378 );
    M_movable_table_v18_narrow.emplace_back( 25.80, 25.798401, 0.644826 );
    M_movable_table_v18_narrow.emplace_back( 27.10, 27.121113, 0.677887 );
    M_movable_table_v18_narrow.emplace_back( 28.50, 28.511642, 0.712643 );
    M_movable_table_v18_narrow.emplace_back( 30.00, 29.973465, 0.749181 );
    M_movable_table_v18_narrow.emplace_back( 31.50, 31.510237, 0.787592 );
    M_movable_table_v18_narrow.emplace_back( 33.10, 33.125802, 0.827972 );
    M_movable_table_v18_narrow.emplace_back( 34.80, 34.824198, 0.870423 );
    M_movable_table_v18_narrow.emplace_back( 36.60, 36.609672, 0.915051 );
    M_movable_table_v18_narrow.emplace_back( 38.50, 38.486690, 0.961966 );
    M_movable_table_v18_narrow.emplace_back( 40.40, 40.459945, 1.011288 );
    M_movable_table_v18_narrow.emplace_back( 42.50, 42.534371, 1.063138 );
    M_movable_table_v18_narrow.emplace_back( 44.70, 44.715155, 1.117646 );
    M_movable_table_v18_narrow.emplace_back( 47.00, 47.007750, 1.174949 );
    M_movable_table_v18_narrow.emplace_back( 49.40, 49.417889, 1.235190 );
    M_movable_table_v18_narrow.emplace_back( 51.90, 51.951598, 1.298519 );
    M_movable_table_v18_narrow.emplace_back( 54.60, 54.615213, 1.365096 );
    M_movable_table_v18_narrow.emplace_back( 57.40, 57.415395, 1.435086 );
    M_movable_table_v18_narrow.emplace_back( 60.30, 60.359145, 1.508664 );
    M_movable_table_v18_narrow.emplace_back( 63.40, 63.453825, 1.586015 );
    M_movable_table_v18_narrow.emplace_back( 66.70, 66.707172, 1.667332 );
    M_movable_table_v18_narrow.emplace_back( 70.10, 70.127322, 1.752818 );
    M_movable_table_v18_narrow.emplace_back( 73.70, 73.722826, 1.842686 );
    M_movable_table_v18_narrow.emplace_back( 77.50, 77.502676, 1.937163 );
    M_movable_table_v18_narrow.emplace_back( 81.50, 81.476324, 2.036484 );
    M_movable_table_v18_narrow.emplace_back( 85.60, 85.653704, 2.140896 );
    M_movable_table_v18_narrow.emplace_back( 90.00, 90.045263, 2.250662 );
    M_movable_table_v18_narrow.emplace_back( 94.60, 94.661983, 2.366057 );
    M_movable_table_v18_narrow.emplace_back( 99.50, 99.515407, 2.487367 );
    M_movable_table_v18_narrow.emplace_back( 104.60, 104.617671, 2.614897 );
    M_movable_table_v18_narrow.emplace_back( 109.90, 109.981533, 2.748965 );
    M_movable_table_v18_narrow.emplace_back( 115.60, 115.620407, 2.889908 );
    M_movable_table_v18_narrow.emplace_back( 121.50, 121.548392, 3.038077 );
    M_movable_table_v18_narrow.emplace_back( 127.70, 127.780311, 3.193842 );
    M_movable_table_v18_narrow.emplace_back( 134.30, 134.331748, 3.357594 );
    M_movable_table_v18_narrow.emplace_back( 141.20, 141.219084, 3.529742 );
    M_movable_table_v18_narrow.emplace_back( 148.40, 148.459541, 3.710715 );

    //
    M_movable_table_v18_normal.emplace_back( 0.00, 0.025845, 0.025845 );
    M_movable_table_v18_normal.emplace_back( 0.10, 0.099701, 0.048011 );
    M_movable_table_v18_normal.emplace_back( 0.20, 0.198705, 0.050994 );
    M_movable_table_v18_normal.emplace_back( 0.30, 0.306504, 0.056805 );
    M_movable_table_v18_normal.emplace_back( 0.40, 0.409145, 0.045835 );
    M_movable_table_v18_normal.emplace_back( 0.50, 0.512382, 0.057401 );
    M_movable_table_v18_normal.emplace_back( 0.60, 0.615888, 0.046106 );
    M_movable_table_v18_normal.emplace_back( 0.70, 0.715560, 0.053567 );
    M_movable_table_v18_normal.emplace_back( 0.80, 0.799079, 0.029952 );
    M_movable_table_v18_normal.emplace_back( 0.90, 0.896113, 0.067083 );
    M_movable_table_v18_normal.emplace_back( 1.00, 1.000704, 0.037508 );
    M_movable_table_v18_normal.emplace_back( 1.10, 1.078643, 0.040430 );
    M_movable_table_v18_normal.emplace_back( 1.20, 1.162652, 0.043579 );
    M_movable_table_v18_normal.emplace_back( 1.30, 1.303835, 0.097604 );
    M_movable_table_v18_normal.emplace_back( 1.50, 1.456015, 0.054575 );
    M_movable_table_v18_normal.emplace_back( 1.60, 1.569415, 0.058825 );
    M_movable_table_v18_normal.emplace_back( 1.70, 1.691648, 0.063407 );
    M_movable_table_v18_normal.emplace_back( 1.80, 1.823400, 0.068345 );
    M_movable_table_v18_normal.emplace_back( 2.00, 1.965414, 0.073668 );
    M_movable_table_v18_normal.emplace_back( 2.10, 2.118489, 0.079406 );
    M_movable_table_v18_normal.emplace_back( 2.30, 2.283485, 0.085591 );
    M_movable_table_v18_normal.emplace_back( 2.50, 2.461333, 0.092257 );
    M_movable_table_v18_normal.emplace_back( 2.70, 2.653032, 0.099442 );
    M_movable_table_v18_normal.emplace_back( 2.90, 2.859661, 0.107187 );
    M_movable_table_v18_normal.emplace_back( 3.10, 3.082384, 0.115536 );
    M_movable_table_v18_normal.emplace_back( 3.30, 3.322453, 0.124534 );
    M_movable_table_v18_normal.emplace_back( 3.60, 3.581219, 0.134233 );
    M_movable_table_v18_normal.emplace_back( 3.90, 3.860139, 0.144688 );
    M_movable_table_v18_normal.emplace_back( 4.20, 4.160782, 0.155956 );
    M_movable_table_v18_normal.emplace_back( 4.50, 4.484841, 0.168103 );
    M_movable_table_v18_normal.emplace_back( 4.80, 4.834139, 0.181195 );
    M_movable_table_v18_normal.emplace_back( 5.20, 5.210642, 0.195308 );
    M_movable_table_v18_normal.emplace_back( 5.60, 5.616468, 0.210519 );
    M_movable_table_v18_normal.emplace_back( 6.00, 6.053902, 0.226915 );
    M_movable_table_v18_normal.emplace_back( 6.50, 6.525405, 0.244588 );
    M_movable_table_v18_normal.emplace_back( 7.00, 7.033631, 0.263638 );
    M_movable_table_v18_normal.emplace_back( 7.60, 7.581439, 0.284171 );
    M_movable_table_v18_normal.emplace_back( 8.20, 8.171913, 0.306303 );
    M_movable_table_v18_normal.emplace_back( 8.80, 8.808376, 0.330159 );
    M_movable_table_v18_normal.emplace_back( 9.50, 9.494408, 0.355873 );
    M_movable_table_v18_normal.emplace_back( 10.20, 10.233872, 0.383590 );
    M_movable_table_v18_normal.emplace_back( 11.00, 11.030928, 0.413466 );
    M_movable_table_v18_normal.emplace_back( 11.90, 11.890062, 0.445668 );
    M_movable_table_v18_normal.emplace_back( 12.80, 12.816110, 0.480379 );
    M_movable_table_v18_normal.emplace_back( 13.80, 13.814282, 0.517793 );
    M_movable_table_v18_normal.emplace_back( 14.90, 14.890195, 0.558120 );
    M_movable_table_v18_normal.emplace_back( 16.00, 16.049905, 0.601590 );
    M_movable_table_v18_normal.emplace_back( 17.30, 17.299939, 0.648444 );
    M_movable_table_v18_normal.emplace_back( 18.60, 18.647330, 0.698948 );
    M_movable_table_v18_normal.emplace_back( 20.10, 20.099662, 0.753384 );
    M_movable_table_v18_normal.emplace_back( 21.60, 21.665107, 0.812061 );
    M_movable_table_v18_normal.emplace_back( 23.30, 23.352476, 0.875308 );
    M_movable_table_v18_normal.emplace_back( 25.20, 25.171263, 0.943480 );
    M_movable_table_v18_normal.emplace_back( 27.10, 27.131705, 1.016962 );
    M_movable_table_v18_normal.emplace_back( 29.20, 29.244835, 1.096168 );
    M_movable_table_v18_normal.emplace_back( 31.50, 31.522545, 1.181541 );
    M_movable_table_v18_normal.emplace_back( 34.00, 33.977651, 1.273565 );
    M_movable_table_v18_normal.emplace_back( 36.60, 36.623971, 1.372755 );
    M_movable_table_v18_normal.emplace_back( 39.40, 39.476398, 1.479671 );
    M_movable_table_v18_normal.emplace_back( 42.50, 42.550983, 1.594914 );
    M_movable_table_v18_normal.emplace_back( 45.80, 45.865031, 1.719133 );
    M_movable_table_v18_normal.emplace_back( 49.40, 49.437190, 1.853026 );
    M_movable_table_v18_normal.emplace_back( 53.30, 53.287563, 1.997347 );
    M_movable_table_v18_normal.emplace_back( 57.40, 57.437820, 2.152909 );
    M_movable_table_v18_normal.emplace_back( 61.90, 61.911315, 2.320586 );
    M_movable_table_v18_normal.emplace_back( 66.70, 66.733226, 2.501324 );
    M_movable_table_v18_normal.emplace_back( 71.90, 71.930687, 2.696137 );
    M_movable_table_v18_normal.emplace_back( 77.50, 77.532947, 2.906123 );
    M_movable_table_v18_normal.emplace_back( 83.50, 83.571534, 3.132464 );
    M_movable_table_v18_normal.emplace_back( 90.00, 90.080432, 3.376433 );
    M_movable_table_v18_normal.emplace_back( 97.00, 97.096270, 3.639404 );
    M_movable_table_v18_normal.emplace_back( 104.60, 104.658531, 3.922856 );
    M_movable_table_v18_normal.emplace_back( 112.70, 112.809772, 4.228385 );
    M_movable_table_v18_normal.emplace_back( 121.50, 121.595865, 4.557708 );
    M_movable_table_v18_normal.emplace_back( 131.00, 131.066256, 4.912682 );
    M_movable_table_v18_normal.emplace_back( 141.20, 141.274240, 5.295302 );

    //
    M_movable_table_v18_wide.emplace_back( 0.00, 0.026170, 0.026170 );
    M_movable_table_v18_wide.emplace_back( 0.10, 0.104789, 0.052449 );
    M_movable_table_v18_wide.emplace_back( 0.20, 0.208240, 0.051001 );
    M_movable_table_v18_wide.emplace_back( 0.30, 0.304589, 0.045348 );
    M_movable_table_v18_wide.emplace_back( 0.40, 0.411152, 0.061214 );
    M_movable_table_v18_wide.emplace_back( 0.50, 0.524658, 0.052292 );
    M_movable_table_v18_wide.emplace_back( 0.60, 0.607289, 0.030340 );
    M_movable_table_v18_wide.emplace_back( 0.70, 0.708215, 0.070586 );
    M_movable_table_v18_wide.emplace_back( 0.80, 0.819755, 0.040954 );
    M_movable_table_v18_wide.emplace_back( 0.90, 0.905969, 0.045261 );
    M_movable_table_v18_wide.emplace_back( 1.00, 1.001251, 0.050021 );
    M_movable_table_v18_wide.emplace_back( 1.10, 1.106553, 0.055281 );
    M_movable_table_v18_wide.emplace_back( 1.20, 1.222930, 0.061095 );
    M_movable_table_v18_wide.emplace_back( 1.30, 1.351547, 0.067521 );
    M_movable_table_v18_wide.emplace_back( 1.50, 1.493690, 0.074622 );
    M_movable_table_v18_wide.emplace_back( 1.60, 1.650783, 0.082470 );
    M_movable_table_v18_wide.emplace_back( 1.80, 1.824397, 0.091143 );
    M_movable_table_v18_wide.emplace_back( 2.00, 2.016271, 0.100730 );
    M_movable_table_v18_wide.emplace_back( 2.20, 2.228324, 0.111323 );
    M_movable_table_v18_wide.emplace_back( 2.50, 2.462678, 0.123032 );
    M_movable_table_v18_wide.emplace_back( 2.70, 2.721681, 0.135971 );
    M_movable_table_v18_wide.emplace_back( 3.00, 3.007923, 0.150271 );
    M_movable_table_v18_wide.emplace_back( 3.30, 3.324268, 0.166075 );
    M_movable_table_v18_wide.emplace_back( 3.70, 3.673885, 0.183542 );
    M_movable_table_v18_wide.emplace_back( 4.10, 4.060271, 0.202845 );
    M_movable_table_v18_wide.emplace_back( 4.50, 4.487293, 0.224178 );
    M_movable_table_v18_wide.emplace_back( 5.00, 4.959226, 0.247755 );
    M_movable_table_v18_wide.emplace_back( 5.50, 5.480792, 0.273812 );
    M_movable_table_v18_wide.emplace_back( 6.00, 6.057212, 0.302609 );
    M_movable_table_v18_wide.emplace_back( 6.70, 6.694254, 0.334434 );
    M_movable_table_v18_wide.emplace_back( 7.40, 7.398295, 0.369607 );
    M_movable_table_v18_wide.emplace_back( 8.20, 8.176381, 0.408478 );
    M_movable_table_v18_wide.emplace_back( 9.00, 9.036297, 0.451438 );
    M_movable_table_v18_wide.emplace_back( 10.00, 9.986653, 0.498917 );
    M_movable_table_v18_wide.emplace_back( 11.00, 11.036958, 0.551388 );
    M_movable_table_v18_wide.emplace_back( 12.20, 12.197725, 0.609378 );
    M_movable_table_v18_wide.emplace_back( 13.50, 13.480571, 0.673467 );
    M_movable_table_v18_wide.emplace_back( 14.90, 14.898335, 0.744296 );
    M_movable_table_v18_wide.emplace_back( 16.40, 16.465207, 0.822575 );
    M_movable_table_v18_wide.emplace_back( 18.20, 18.196868, 0.909086 );
    M_movable_table_v18_wide.emplace_back( 20.10, 20.110649, 1.004696 );
    M_movable_table_v18_wide.emplace_back( 22.20, 22.225705, 1.110360 );
    M_movable_table_v18_wide.emplace_back( 24.50, 24.563203, 1.227138 );
    M_movable_table_v18_wide.emplace_back( 27.10, 27.146537, 1.356197 );
    M_movable_table_v18_wide.emplace_back( 30.00, 30.001564, 1.498830 );
    M_movable_table_v18_wide.emplace_back( 33.10, 33.156856, 1.656462 );
    M_movable_table_v18_wide.emplace_back( 36.60, 36.643992, 1.830674 );
    M_movable_table_v18_wide.emplace_back( 40.40, 40.497875, 2.023208 );
    M_movable_table_v18_wide.emplace_back( 44.70, 44.757073, 2.235990 );
    M_movable_table_v18_wide.emplace_back( 49.40, 49.464215, 2.471151 );
    M_movable_table_v18_wide.emplace_back( 54.60, 54.666412, 2.731045 );
    M_movable_table_v18_wide.emplace_back( 60.30, 60.415729, 3.018271 );
    M_movable_table_v18_wide.emplace_back( 66.70, 66.769707, 3.335706 );
    M_movable_table_v18_wide.emplace_back( 73.70, 73.791938, 3.686525 );
    M_movable_table_v18_wide.emplace_back( 81.50, 81.552704, 4.074240 );
    M_movable_table_v18_wide.emplace_back( 90.00, 90.129677, 4.502732 );
    M_movable_table_v18_wide.emplace_back( 99.50, 99.608697, 4.976288 );
    M_movable_table_v18_wide.emplace_back( 109.90, 110.084635, 5.499649 );
    M_movable_table_v18_wide.emplace_back( 121.50, 121.662338, 6.078053 );
    M_movable_table_v18_wide.emplace_back( 134.30, 134.457678, 6.717287 );
    M_movable_table_v18_wide.emplace_back( 148.40, 148.598714, 7.423750 );

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
                               []( const DataEntry & lhs,
                                   const DataEntry & rhs )
                               {
                                   return lhs.M_seen_dist < rhs.M_seen_dist;
                               } );
    if ( it == M_static_table.end() )
    {
        std::cerr << "(ObjectTable::getStaticObjInfo) illegal distance = "
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
                               []( const DataEntry & lhs,
                                   const DataEntry & rhs )
                               {
                                   return lhs.M_seen_dist < rhs.M_seen_dist;
                               } );
    if ( it == M_movable_table.end() )
    {
        std::cerr << "(ObjectTable::getMovableObjInfo) illegal distance = "
                  << see_dist << std::endl;
        return false;
    }

    *ave = it->M_average;
    *err = it->M_error;

    return true;
}

/*-------------------------------------------------------------------*/
bool
ObjectTable::getLandmarkDistanceRangeV18( const ViewWidth::Type view_width,
                                          const double quant_dist,
                                          double * mean_dist,
                                          double * dist_error ) const
{
    const std::vector< DataEntry > * table = ( view_width == ViewWidth::NARROW
                                               ? &M_static_table_v18_narrow
                                               : view_width == ViewWidth::NORMAL
                                               ? &M_static_table_v18_normal
                                               : &M_static_table_v18_wide );

    std::vector< DataEntry >::const_iterator
        it = std::lower_bound( table->begin(),
                               table->end(),
                               DataEntry( quant_dist - 0.001 ),
                               []( const DataEntry & lhs,
                                   const DataEntry & rhs )
                               {
                                   return lhs.M_seen_dist < rhs.M_seen_dist;
                               } );
    if ( it == table->end() )
    {
        std::cerr << "(ObjectTable::getLandmarkDistanceRangeV18) illegal distance = " << quant_dist << std::endl;
        return false;
    }

    *mean_dist = it->M_average;
    *dist_error = it->M_error;

    return true;
}

/*-------------------------------------------------------------------*/
bool
ObjectTable::getDistanceRangeV18( const ViewWidth::Type view_width,
                                  const double quant_dist,
                                  double * mean_dist,
                                  double * dist_error ) const
{
    const std::vector< DataEntry > * table = ( view_width == ViewWidth::NARROW
                                               ? &M_movable_table_v18_narrow
                                               : view_width == ViewWidth::NORMAL
                                               ? &M_movable_table_v18_normal
                                               : &M_movable_table_v18_wide );

    std::vector< DataEntry >::const_iterator
        it = std::lower_bound( table->begin(),
                               table->end(),
                               DataEntry( quant_dist - 0.001 ),
                               []( const DataEntry & lhs,
                                   const DataEntry & rhs )
                               {
                                   return lhs.M_seen_dist < rhs.M_seen_dist;
                               } );
    if ( it == table->end() )
    {
        std::cerr << "(ObjectTable::getDistanceRangeV18) illegal distance = " << quant_dist << std::endl;
        return false;
    }

    *mean_dist = it->M_average;
    *dist_error = it->M_error;

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

        table.emplace_back( see_dist, // quantized dist
                            (max_dist + min_dist) * 0.5,   // average
                            (max_dist - min_dist) * 0.5 ); // error
    }
}

}
