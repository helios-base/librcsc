// -*-c++-*-

/*!
  \file host_address.cpp
  \brief address class source File.
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

#include "host_address.h"

#include <iostream>
#include <cstring>

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

namespace rcsc {

class HostAddress::Impl {
public:
    struct sockaddr_in addr_;

    Impl()
      {
          clear();
      }

    void clear()
      {
          std::memset( reinterpret_cast< char * >( &addr_ ),
                       0,
                       sizeof( struct sockaddr_in ) );
      }
};

/*-------------------------------------------------------------------*/
/*!

*/
HostAddress::HostAddress()
    : M_impl( new Impl() )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
HostAddress::HostAddress( const HostAddress & addr )
    : M_impl( new Impl() )
{
    M_impl->addr_ = addr.M_impl->addr_;
}

/*-------------------------------------------------------------------*/
/*!

*/
HostAddress::HostAddress( const AddrType & addr )
    : M_impl( new Impl() )
{
    M_impl->addr_ = addr;
}

/*-------------------------------------------------------------------*/
/*!

*/
HostAddress::~HostAddress()
{
    M_impl->clear();
}

/*-------------------------------------------------------------------*/
/*!

*/
const HostAddress &
HostAddress::operator=( const HostAddress & addr )
{
    if ( this != &addr )
    {
        M_impl->addr_ = addr.M_impl->addr_;
    }

    return *this;
}

/*-------------------------------------------------------------------*/
/*!

*/
const HostAddress &
HostAddress::operator=( const AddrType & addr )
{
    M_impl->addr_ = addr;
    return *this;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
HostAddress::clear()
{
    M_impl->clear();
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
HostAddress::isNull() const
{
    return M_impl->addr_.sin_addr.s_addr == 0;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
HostAddress::equals( const HostAddress & addr ) const
{
    return this->toIPV4Address() == addr.toIPV4Address()
        && this->portNumber() == addr.portNumber();
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
HostAddress::equals( const AddrType & addr ) const
{
    return M_impl->addr_.sin_addr.s_addr == addr.sin_addr.s_addr
        && M_impl->addr_.sin_port == addr.sin_port;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
HostAddress::setAddress( const AddrType & addr )
{
    M_impl->addr_ = addr;
}

/*-------------------------------------------------------------------*/
/*!

*/
HostAddress::PortNumber
HostAddress::portNumber() const
{
    return ntohs( M_impl->addr_.sin_port );
}

/*-------------------------------------------------------------------*/
/*!

*/
HostAddress::IPV4Address
HostAddress::toIPV4Address() const
{
    return IPV4Address( ntohl( M_impl->addr_.sin_addr.s_addr ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
std::string
HostAddress::toHostName() const
{
    return std::string( ::inet_ntoa( M_impl->addr_.sin_addr ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
const HostAddress::AddrType &
HostAddress::toAddress() const
{
    return M_impl->addr_;
}

}
