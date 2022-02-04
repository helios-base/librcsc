// -*-c++-*-

/*!
  \file udp_socket.h
  \brief UDP connection socket class Header File.
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

#ifndef RCSC_NET_UDP_SOCKET_H
#define RCSC_NET_UDP_SOCKET_H

#include <rcsc/net/abstract_socket.h>

#include <boost/scoped_ptr.hpp>

#include <cstddef>

namespace rcsc {

/*!
  \class UDPSocket
  \brief UDP/IP connection socket class
*/
class UDPSocket
    : public AbstractSocket {
private:
    //! not used
    UDPSocket() = delete;
public:
    /*!
      \brief constructor for server socket
      \param port port number to receive packet.
    */
    explicit
    UDPSocket( const int port );

    /*!
      \brief constructor for client socket
      \param hostname remote host name (or IP address)
      \param port port number to send packet
     */
    UDPSocket( const char * hostname,
               const int port );

    /*!
      \brief destructor. close socket automatically
     */
    ~UDPSocket();

public:
     /*!
      \brief send datagram packet to the connected host.
      \param data the pointer to the data to be sent.
      \param len the length of data.
      \return the length of sent data if successfuly sent, otherwise -1.
     */
    int writeDatagram( const char * data,
                       const size_t len );

     /*!
      \brief send datagram packet to the specified host.
      \param data the pointer to the data to be sent.
      \param len the length of data.
      \param dest the target host address
      \return the length of sent data if successfuly sent, otherwise -1.
     */
    int writeDatagram( const char * data,
                       const size_t len,
                       const HostAddress & dest );

    /*!
      \brief receive datagram packet from the connected remote host.
      \param buf buffer to receive data
      \param len maximum length of the buffer array
      \retval 0 error occured and errno is EWOULDBLOCK
      \retval -1 error occured
      \return the length of received data.
     */
    int readDatagram( char * buf,
                      const size_t len );

    /*!
      \brief receive datagram packet from the connected remote host.
      \param buf buffer to receive data
      \param len maximum length of the buffer array
      \param addr the source host address is set to this variable
      \retval 0 error occured and errno is EWOULDBLOCK
      \retval -1 error occured
      \return the length of received data.
     */
    int readDatagram( char * buf,
                      const size_t len,
                      HostAddress * from );

};

} // end namespace

#endif
