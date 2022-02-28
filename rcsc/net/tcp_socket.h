// -*-c++-*-

/*!
  \file tcp_socket.h
  \brief TCP connection socket class Header File.
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

#ifndef RCSC_NET_TCP_SOCKET_H
#define RCSC_NET_TCP_SOCKET_H

#include <rcsc/net/abstract_socket.h>

#include <cstddef>

namespace rcsc {

/*!
  \class TCPSocket
  \brief TCP/IP connection socket class
*/
class TCPSocket
    : public AbstractSocket {
private:
    //! not used
    TCPSocket() = delete;

public:
    /*!
      \brief constructor for client socket
      \param hostname remote host name (or IP address)
      \param port port number to send packet
     */
    TCPSocket( const char * hostname,
               const int port );

    /*!
      \brief destructor. close socket automatically
     */
    ~TCPSocket();

    /*!
      \brief make connection
      \return -1 on error, otherwise 0
     */
    int connect();

    /*!
      \brief send stream data to the connected host.
      \param data the pointer to the data to be sent.
      \param len the length of data.
      \return the length of sent data if successfuly sent, otherwise -1.
     */
    int writeToStream( const char * data,
                       const std::size_t len );

    /*!
      \brief receive stream data from the connected remote host.
      \param buf buffer to receive data
      \param len maximal length of buffer buf
      \retval 0 error occured and errno is EWOULDBLOCK
      \retval -1 error occured
      \return the length of received data.
     */
    int readFromStream( char * buf,
                        const std::size_t len );

};

} // end namespace

#endif
