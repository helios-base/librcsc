// -*-c++-*-

/*!
  \file abstract_socket.cpp
  \brief basic socket class Source File.
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

#include "abstract_socket.h"

#include <iostream>

#include <cstdio> // std::perror()
#include <cstring> // memset(), memcpy()
#include <cerrno> // errno

#ifdef HAVE_NETDB_H
#include <netdb.h> // gethostbyname(), getaddrinfo(), freeaddrinfo()
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h> // close()
#endif
#include <fcntl.h> // fcntl()
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
AbstractSocket::AbstractSocket()
    : M_fd( -1 ),
      M_socket_type( UNKNOWN_TYPE ),
      M_local_address(),
      M_peer_address()
{

}

/*-------------------------------------------------------------------*/
/*!

 */
AbstractSocket::~AbstractSocket()
{
    this->close();
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
AbstractSocket::open( const SocketType type )
{
#ifdef HAVE_SOCKET
    switch ( type ) {
    case STREAM_TYPE:
        M_socket_type = SOCK_STREAM;
        break;
    case DATAGRAM_TYPE:
        M_socket_type = SOCK_DGRAM;
        break;
    default:
        std::cerr << "(AbstractSocket::open) ***ERROR*** unknown socket type."
                  << std::endl;
        return false;
    }

    M_fd = ::socket( AF_INET, M_socket_type, 0 );
#endif

    if ( fd() == -1 )
    {
        std::cerr << "(AbstractSocket::open) ***ERROR*** failed to open a socket."
                  << std::endl;
        std::perror( "socket" );
        std::cerr << errno << std::endl;
        return false;
    }

    ::fcntl( fd(), F_SETFD, FD_CLOEXEC ); // close on exec
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
AbstractSocket::bind( const HostAddress::PortNumber port )
{
    if ( fd() == -1 )
    {
        return false;
    }

    HostAddress::AddrType my_addr;
    std::memset( reinterpret_cast< char * >( &my_addr ),
                 0,
                 sizeof( HostAddress::AddrType ) );
    my_addr.sin_family = AF_INET; // internet connection
    my_addr.sin_addr.s_addr = htonl( INADDR_ANY );
    my_addr.sin_port = htons( port );

    if ( ::bind( fd(),
                 reinterpret_cast< struct sockaddr * >( &my_addr ),
                 sizeof( HostAddress::AddrType ) ) < 0 )
    {
        std::cerr << "(AbstractSocket::bind) ***ERROR*** failed to bind."
                  << std::endl;
        this->close();
        return false;
    }

    M_local_address.setAddress( my_addr );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
AbstractSocket::setPeerAddress( const char * hostname,
                                const HostAddress::PortNumber port )
{
    HostAddress::AddrType dest_addr;

#ifdef HAVE_GETADDRINFO
    struct addrinfo hints;
    std::memset( &hints, 0, sizeof( hints ) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = M_socket_type;
    hints.ai_protocol = 0;

    struct addrinfo * res;
    int err = ::getaddrinfo( hostname, NULL, &hints, &res );
    if ( err != 0 )
    {
        std::cerr << "(AbstractSocket::setAddr) ***ERROR*** failed to resolve the host ["
                  << hostname << "]" << std::endl;
        std::cerr << "(AbstractSocket::setAddr) error=" << err << ' '
                  << gai_strerror( err ) << std::endl;
        this->close();
        return false;

    }

    dest_addr.sin_addr.s_addr = (reinterpret_cast< struct sockaddr_in * >(res->ai_addr))->sin_addr.s_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons( port );

    M_peer_address.setAddress( dest_addr );

    ::freeaddrinfo( res );
    return true;

#endif

#ifdef HAVE_GETHOSTBYNAME
#ifdef HAVE_INET_ADDR
    dest_addr.sin_addr.s_addr = ::inet_addr( hostname );
    if ( dest_addr.sin_addr.s_addr == 0xffffffff )
#endif
    {
        struct hostent * host_entry = ::gethostbyname( hostname );
        if ( ! host_entry )
        {
            std::cerr << hstrerror( h_errno ) << std::endl;
            std::cerr << "(AbstractSocket::setAddr) host not found ["
                      << hostname << "]" << std::endl;
            this->close();
            return false;
        }

        std::memcpy( &(dest_addr.sin_addr.s_addr),
                     host_entry->h_addr_list[0],
                     host_entry->h_length );
    }

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons( port );

    M_peer_address.setAddress( dest_addr );

    return true;
#endif

    std::cerr << "(AbstractSocket::setAddr) ***ERROR*** no getaddrinfo or gethostbyname."
              << "failed to resolve the host [" << hostname << "]" << std::endl;
    this->close();
    return false;
}

/*-------------------------------------------------------------------*/
/*!

 */
int
AbstractSocket::connectToPresetAddr()
{
    int ret = ::connect( fd(),
                         reinterpret_cast< const sockaddr * >( &(M_peer_address.toAddress()) ),
                         sizeof( HostAddress::AddrType ) );
    if ( ret == -1 )
    {
        std::perror( "connect" );
    }

    return ret;
}

/*-------------------------------------------------------------------*/
/*!

 */
int
AbstractSocket::setNonBlocking()
{
    int flags = ::fcntl( fd(), F_GETFL, 0 );
    if ( flags == -1 )
    {
        return flags;
    }

    return ::fcntl( fd(), F_SETFL, O_NONBLOCK | flags );
}

/*-------------------------------------------------------------------*/
/*!

 */
int
AbstractSocket::close()
{
    if ( fd() != -1 )
    {
        int ret = ::close( fd() );
        M_fd = -1;
        M_peer_address.clear();
        return ret;
    }

    return 0;
}

} // end namespace
