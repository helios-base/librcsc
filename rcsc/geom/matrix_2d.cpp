// -*-c++-*-

/*!
  \file matrix_2d.cpp
  \brief 2D transform matrix class Source File.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "matrix_2d.h"

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
Matrix2D
Matrix2D::inverted() const
{
    double determinant = det();
    if ( determinant == 0.0 )
    {
        // never invertible
        return Matrix2D(); // default matrix
    }

    double dinv = 1.0 / determinant;
    return Matrix2D( M_22 * dinv, -M_12 * dinv,
                     -M_21 * dinv, M_11 * dinv,
                     (M_12*M_dy - M_dx*M_22) * dinv,
                     (M_dx*M_21 - M_11*M_dy) * dinv );
}

/*-------------------------------------------------------------------*/
/*!

 */
Matrix2D &
Matrix2D::rotate( const AngleDeg & angle )
{
    // rotate matrix
    // R = ( cona, -sina, 0 )
    //     ( sina,  cosa, 0 )
    //     (    0,     0, 1 )

    // this = R * this
    // *this = create_rotation(angle) * *this;

    double sina = angle.sin();
    double cosa = angle.cos();

    double tm11 = M_11*cosa - M_21*sina;
    double tm12 = M_12*cosa - M_22*sina;
    double tm21 = M_11*sina + M_21*cosa;
    double tm22 = M_12*sina + M_22*cosa;
    double tdx = M_dx*cosa - M_dy*sina;
    double tdy = M_dx*sina + M_dy*cosa;
    M_11 = tm11; M_12 = tm12; M_dx = tdx;
    M_21 = tm21; M_22 = tm22; M_dy = tdy;
    return *this;
}


}
