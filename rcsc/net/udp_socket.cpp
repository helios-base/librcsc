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

#include <cstdio>
#include <cerrno>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h> // struct sockaddr_in, struct in_addr, htons
#endif

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
UDPSocket::UDPSocket( const int port )
    : AbstractSocket()
{
    if ( open( AbstractSocket::DATAGRAM_TYPE )
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
    : AbstractSocket()
{
    if ( open( AbstractSocket::DATAGRAM_TYPE )
         && bind( 0 )
         && setPeerAddress( hostname, port )
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
UDPSocket::writeDatagram( const char * data,
                          const size_t len )
{
    return writeDatagram( data, len, M_peer_address );
}

/*-------------------------------------------------------------------*/
/*!

 */
int
UDPSocket::writeDatagram( const char * data,
                          const size_t len,
                          const HostAddress & dest )
{
    int n = ::sendto( fd(), data, len, 0,
                      reinterpret_cast< const sockaddr * >( &(dest.toAddress()) ),
                      sizeof( HostAddress::AddrType ) );
    if ( n != static_cast< int >( len ) )
    {
        std::perror( "sendto" );
        return -1;
    }

    return len;
}

/*-------------------------------------------------------------------*/
/*!

*/
int
UDPSocket::readDatagram( char * buf,
                         const size_t len )
{
    return readDatagram( buf, len, &M_peer_address );
}

/*-------------------------------------------------------------------*/
/*!

 */
int
UDPSocket::readDatagram( char * buf,
                         const size_t len,
                         HostAddress * from )
{
    HostAddress::AddrType from_addr;
    socklen_t from_size = sizeof( HostAddress::AddrType );
    int n = ::recvfrom( fd(), buf, len, 0,
                        reinterpret_cast< struct sockaddr * >( &from_addr ),
                        &from_size );

    if ( n == -1 )
    {
        if ( errno == EWOULDBLOCK )
        {
            return 0;
        }

        std::perror( "recvfrom" );
        return -1;
    }

    if ( from )
    {
        from->setAddress( from_addr );
    }

    return n;
}

} // end namespace
