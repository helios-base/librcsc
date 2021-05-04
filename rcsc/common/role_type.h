// -*-c++-*-

/*!
  \file role_type.h
  \brief player role type Header File
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

#ifndef RCSC_COMMON_ROLE_TYPE_H
#define RCSC_COMMON_ROLE_TYPE_H

namespace rcsc {

/*!
  \class RoleType
  \brief player's role type information class
 */
class RoleType {
public:
    /*!
      \enum Type
      \brief role type
     */
    enum Type {
        Goalie = 0,
        Defender,
        MidFielder,
        Forward,
        Unknown,
    };

    /*!
      \enum Side
      \brief position type (y position)
     */
    enum Side {
        Center = 0,
        Left = -1,
        Right = 1,
    };

private:

    Type M_type; //!< role type
    Side M_side; //< role position type

public:

    /*!
      \brief default constructor creates an illegal type
     */
    RoleType()
        : M_type( Unknown ),
          M_side( Center )
      { }

    RoleType( const Type t,
              const Side s )
        : M_type( t ),
          M_side( s )
      { }

    void setType( Type t ) { M_type = t; }
    void setSide( Side s ) { M_side = s; }

    Type type() const { return M_type; }
    Side side() const { return M_side; }

    bool isGoalie() const { return M_type == Goalie; }
    bool isDefender() const { return M_type == Defender; }
    bool isMidFielder() const { return M_type == MidFielder; }
    bool isForward() const { return M_type == Forward; }

    bool isCenter() const { return M_side == Center; }
    bool isLeft() const { return M_side == Left; }
    bool isRight() const { return M_side == Right; }
};

}

#endif
