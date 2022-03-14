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

#include <string>

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
      \brief default constructor
     */
    RoleType()
        : M_type( Unknown ),
          M_side( Center )
      { }

    /*!
      \brief create with given values
      \param t role type value
      \param s side type value
     */
    RoleType( const Type t,
              const Side s )
        : M_type( t ),
          M_side( s )
      { }

    /*!
      \brief set the role type
      \param t role type value
     */
    void setType( Type t ) { M_type = t; }

    /*!
      \brief set the side type
      \param t side type value
     */
    void setSide( Side s ) { M_side = s; }

    /*!
      \brief get the role type
      \return role type value
     */
    Type type() const { return M_type; }

    /*!
      \brief get the side type
      \return side type value
     */
    Side side() const { return M_side; }

    /*!
      \brief check if goalie type or not
      \return true if goalie
     */
    bool isGoalie() const { return M_type == Goalie; }

    /*!
      \brief check if defender type or not
      \return true if defender
     */
    bool isDefender() const { return M_type == Defender; }

    /*!
      \brief check if midfielder type or not
      \return true if midfielder
     */
    bool isMidFielder() const { return M_type == MidFielder; }

    /*!
      \brief check if forward type or not
      \return true if forward
     */
    bool isForward() const { return M_type == Forward; }

    /*!
      \brief check if center type or not
      \return true if center type
     */
    bool isCenter() const { return M_side == Center; }

    /*!
      \brief check if left type or not
      \return true if left type
     */
    bool isLeft() const { return M_side == Left; }

    /*!
      \brief check if right type or not
      \return true if right type
     */
    bool isRight() const { return M_side == Right; }

    /*!
      \brief create a string value corresponding to the given role type
      \return string value
     */
    static std::string to_string( const Type t );

    /*!
      \brief create a string value corresponding to the given side type
      \return string value
     */
    static std::string to_string( const Side s );

    /*!
      \brief create a role type value from the given string.
      \return role type value. Unknown for unsupported values
     */
    static Type to_type( const std::string & str );

    /*!
      \brief create a side type value from the given string.
      \return role type value. Center for unsupported values
     */
    static Side to_side( const std::string & str );

};

}

#endif
