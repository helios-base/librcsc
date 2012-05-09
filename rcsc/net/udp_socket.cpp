// -*-c++-*-

/*!
  \file udp_socket.cpp
  \brief UDP connection socket class Source File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA, Hiroki SHIMORA

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

#include "udp_socket.h"

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
UDPSocket::UDPSocket( const int port )
    : BasicSocket()
{
    if ( open( BasicSocket::DATAGRAM_TYPE )
         && bind( port )
         && setNonBlocking() != -1 )
    {
        return;
    }

    this->close();
}

/*-------------------------------------------------------------------*/
/*!

*/
UDPSocket::UDPSocket( const char * hostname,
                      const int port )
    : BasicSocket()
{
    if ( open( BasicSocket::DATAGRAM_TYPE )
         && bind()
         && setAddr( hostname, port )
         && setNonBlocking() != -1 )
    {
        return;
    }

    this->close();
}

/*-------------------------------------------------------------------*/
/*!

*/
UDPSocket::~UDPSocket()
{
}

/*-------------------------------------------------------------------*/
/*!

*/
int
UDPSocket::send( const char * data,
                 const std::size_t len )
{
    return BasicSocket::sendDatagramPacket( data, len );
}

/*-------------------------------------------------------------------*/
/*!

*/
int
UDPSocket::receive( char * buf,
                    const std::size_t len )
{
    return BasicSocket::receiveDatagramPacket( buf, len, true );
}

} // end namespace
