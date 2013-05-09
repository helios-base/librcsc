// -*-c++-*-

/*!
  \file tcp_socket.cpp
  \brief TCP connection socket class Source File.
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

#include "tcp_socket.h"

#include <cstdio>
#include <cerrno>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h> // socket(), getaddrinfo(), freeaddrinfo()
                       // connect(), send(), recv(), sendto(), recvfrom(),
                       // struct sockaddr_in, SOCK_STREAM, SOCK_DGRAM
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h> // socket(), getaddrinfo(), freeaddrinfo()
                        // connect(), send(), recv(), sendto(), recvfrom(),
                        // struct sockaddr_in, SOCK_STREAM, SOCK_DGRAM
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h> // inet_addr()
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h> // struct sockaddr_in, struct in_addr, htons
#endif

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
TCPSocket::TCPSocket( const char * hostname,
                      const int port )
    : AbstractSocket()
{
    if ( open( AbstractSocket::STREAM_TYPE )
         && bind( 0 )
         && setPeerAddress( hostname, port )
         && connectToPresetAddr() != -1 )
    {
        return;
    }

    this->close();
}

/*-------------------------------------------------------------------*/
/*!

*/
TCPSocket::~TCPSocket()
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
TCPSocket::connect()
{
    return AbstractSocket::connectToPresetAddr();
}


/*-------------------------------------------------------------------*/
/*!

 */
int
TCPSocket::writeToStream( const char * msg,
                          const size_t len )
{
    int n = ::send( fd(), msg, len, 0 );

    if ( n == -1 )
    {
        std::perror( "send" );
    }

    return n;
}

/*-------------------------------------------------------------------*/
/*!

 */
int
TCPSocket::readFromStream( char * buf,
                           const size_t len )
{
    int n = ::recv( fd(), buf, len, 0 );
    //std::cerr << "receive: " << n << " bytes" << std::endl;
    if ( n == -1 )
    {
        if ( errno == EWOULDBLOCK )
        {
            return 0;
        }

        std::perror( "recv" );
        return -1;
    }

    return n;
}

} // end namespace
