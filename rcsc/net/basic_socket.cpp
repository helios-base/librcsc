// -*-c++-*-

/*!
  \file basic_socket.cpp
  \brief socket class Source File.
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

#include "basic_socket.h"

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

/*!
  \struct AddrImpl
  \brief Pimpl ideom. addres implementation class
*/
struct AddrImpl {
    typedef struct sockaddr_in AddrType; //!< socket address type

    AddrType addr_; //!< socket address
    int socket_type_; //!< socket type {SOCK_STREAM|SOCK_DGRAM}
};


/*-------------------------------------------------------------------*/
/*!

*/
BasicSocket::BasicSocket()
    : M_fd( -1 )
    , M_dest( new AddrImpl )
{
    std::memset( reinterpret_cast< char * >( &(M_dest->addr_) ),
                 0,
                 sizeof( AddrImpl::AddrType ) );
    M_dest->socket_type_ = -1;
}

/*-------------------------------------------------------------------*/
/*!

*/
BasicSocket::~BasicSocket()
{
    this->close();
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
BasicSocket::open( const SocketType type )
{
#ifdef HAVE_SOCKET
    // create socket

    switch( type ) {
    case BasicSocket::STREAM_TYPE:
        M_dest->socket_type_ = SOCK_STREAM;
        break;
    case BasicSocket::DATAGRAM_TYPE:
        M_dest->socket_type_ = SOCK_DGRAM;
        break;
    default:
        std::cerr << "(BasicSocket::open) ***ERROR*** unknown socket type."
                  << std::endl;
        return -1;
    }

    M_fd = ::socket( AF_INET, M_dest->socket_type_, 0 );
#endif

    if ( fd() == -1 )
    {
        std::cerr << "(BasicSocket::open) ***ERROR*** failed to open a socket."
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
BasicSocket::bind( const int port )
{
    if ( fd() == -1 )
    {
        return false;
    }

    AddrImpl::AddrType my_addr;
    std::memset( reinterpret_cast< char * >( &my_addr ),
                 0,
                 sizeof( AddrImpl::AddrType ) );
    my_addr.sin_family = AF_INET; // internet connection
    my_addr.sin_addr.s_addr = htonl( INADDR_ANY );
    my_addr.sin_port = htons( port );

    if ( ::bind( fd(),
                 reinterpret_cast< struct sockaddr * >( &my_addr ),
                 sizeof( AddrImpl::AddrType ) ) < 0 )
    {
        std::cerr << "(BasicSocket::bind) ***ERROR*** failed to bind."
                  << std::endl;
        this->close();
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
BasicSocket::setAddr( const char * hostname,
                      const int port )
{
#ifdef HAVE_GETADDRINFO
    struct addrinfo hints;
    std::memset( &hints, 0, sizeof( hints ) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = M_dest->socket_type_;
    hints.ai_protocol = 0;

    struct addrinfo * res;
    int err = ::getaddrinfo( hostname, NULL, &hints, &res );
    if ( err != 0 )
    {
        std::cerr << "(BasicSocket::setAddr) ***ERROR*** failed to resolve the host ["
                  << hostname << "]" << std::endl;
        std::cerr << "(BasicSocket::setAddr) error=" << err << ' '
                  << gai_strerror( err ) << std::endl;
        this->close();
        return false;

    }

    M_dest->addr_.sin_addr.s_addr
        = (reinterpret_cast< struct sockaddr_in * >(res->ai_addr))->sin_addr.s_addr;
    M_dest->addr_.sin_family = AF_INET;
    M_dest->addr_.sin_port = htons( port );

    ::freeaddrinfo( res );
    return true;

#endif

#ifdef HAVE_GETHOSTBYNAME
#ifdef HAVE_INET_ADDR
    M_dest->addr_.sin_addr.s_addr = ::inet_addr( hostname );
    if ( M_dest->addr_.sin_addr.s_addr == 0xffffffff )
#endif
    {
        struct hostent * host_entry = ::gethostbyname( hostname );
        if ( ! host_entry )
        {
            std::cerr << hstrerror( h_errno ) << std::endl;
            std::cerr << "(BasicSocket::setAddr) host not found ["
                      << hostname << "]" << std::endl;
            this->close();
            return false;
        }

        std::memcpy( &(M_dest->addr_.sin_addr.s_addr),
                     host_entry->h_addr_list[0],
                     host_entry->h_length );
    }

    M_dest->addr_.sin_family = AF_INET;
    M_dest->addr_.sin_port = htons( port );

    return true;
#endif

    std::cerr << "(BasicSocket::setAddr) ***ERROR*** no getaddrinfo or gethostbyname."
              << "failed to resolve the host [" << hostname << "]" << std::endl;
    this->close();
    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
int
BasicSocket::connectToPresetAddr()
{
    int ret = ::connect( fd(),
                         reinterpret_cast< const sockaddr * >( &(M_dest->addr_) ),
                         sizeof( AddrImpl::AddrType ) );
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
BasicSocket::setNonBlocking()
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
BasicSocket::close()
{
    if ( fd() != -1 )
    {
        int ret = ::close( fd() );
        M_fd = -1;
        std::memset( reinterpret_cast< char * >( &(M_dest->addr_) ),
                     0,
                     sizeof( AddrImpl::AddrType ) );
        return ret;
    }

    return 0;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::string
BasicSocket::getPeerName() const
{
    if ( fd() != 0
         && M_dest )
    {
        return std::string( ::inet_ntoa( M_dest->addr_.sin_addr ) );
    }

    return std::string();
}

/*-------------------------------------------------------------------*/
/*!

*/
int
BasicSocket::getPeerPort() const
{
    if ( fd() != 0
         && M_dest )
    {
        return M_dest->addr_.sin_port;
    }

    return 0;
}

/*-------------------------------------------------------------------*/
/*!

*/
int
BasicSocket::writeToStream( const char * msg,
                            const std::size_t len )
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
BasicSocket::readFromStream( char * buf,
                             const std::size_t len )
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


/*-------------------------------------------------------------------*/
/*!

*/
int
BasicSocket::sendDatagramPacket( const char * data,
                                 const std::size_t len )
{
    int n = ::sendto( fd(), data, len, 0,
                      reinterpret_cast< const sockaddr * >( &(M_dest->addr_) ),
                      sizeof( AddrImpl::AddrType ) );

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
BasicSocket::receiveDatagramPacket( char * buf,
                                    const std::size_t len,
                                    const bool overwrite_dest_addr )
{
    AddrImpl::AddrType from_addr;
    socklen_t from_size = sizeof( AddrImpl::AddrType );
    int n = ::recvfrom( fd(), buf, len, 0,
                        reinterpret_cast< struct sockaddr * >( &from_addr ),
                        &from_size );
    //std::cerr << "receive: " << n << " bytes" << std::endl;
    if ( n == -1 )
    {
        if ( errno == EWOULDBLOCK )
        {
            return 0;
        }

        std::perror( "recvfrom" );
        return -1;
    }

    if ( overwrite_dest_addr
         && from_addr.sin_port != 0
         && from_addr.sin_port != M_dest->addr_.sin_port )
    {
        //std::cerr << "dest port = " << from.sin_port << std::endl;
        M_dest->addr_ = from_addr;
        //M_dest->addr_.sin_port = from_addr.sin_port;
    }

    return n;
}

} // end namespace
