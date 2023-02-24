// -*-c++-*-

/*!
  \file serializer_v6.h
  \brief v6 format rcg serializer class Header File.
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

#ifndef RCSC_RCG_SERIALIZER_V6_H
#define RCSC_RCG_SERIALIZER_V6_H

#include <rcsc/rcg/serializer_v5.h>

namespace rcsc {
namespace rcg {

/*!
  \class SerializerV6
  \brief rcg data serializer interface class
*/
class SerializerV6
    : public SerializerV5 {
public:

    /*!
      \brief constructor
    */
    SerializerV6()
        : SerializerV5()
      { }

    /*!
      \brief destructor
    */
    ~SerializerV6()
      { }

    /*!
      \brief write header
      \param os reference to the output stream
      \return serialization result
    */
    virtual
    std::ostream & serializeHeader( std::ostream & os ) override;

    /*!
      \brief write ShowInfoT
      \param os reference to the output stream
      \param show network byte order data
      \return reference to the output stream
     */
    virtual
    std::ostream & serialize( std::ostream & os,
                              const ShowInfoT & show ) override;

};

} // end of namespace rcg
} // end of namespace rcsc

#endif
