// -*-c++-*-

/*!
  \file abstract_socket.h
  \brief basic socket class Header File.
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

#ifndef RCSC_NET_ABSTRACT_SOCKET_H
#define RCSC_NET_ABSTRACT_SOCKET_H

#include <rcsc/net/host_address.h>

#include <boost/scoped_ptr.hpp>

#include <string>
#include <cstddef>

namespace rcsc {

/*!
  \class AbstractSocket
  \brief socket class
*/
class AbstractSocket {
public:
    /*!
      \enum SocketType
      \brief socket type definition
     */
    enum SocketType {
        DATAGRAM_TYPE,
        STREAM_TYPE,
        UNKNOWN_TYPE,
    };

private:
    //! socket file descriptor
    int M_fd;
    //! socket type
    int M_socket_type;

protected:
    //! local address
    HostAddress M_local_address;
    //! destination address
    HostAddress M_peer_address;

protected:
    /*!
      \brief protected constructor to prevent instantiation
    */
    AbstractSocket();

public:
    /*!
      \brief destructor. close socket automatically
    */
    virtual
    ~AbstractSocket();

    /*!
      \brief returns socket file descriptor
      \return socket file descriptor
    */
    int fd() const
      {
          return M_fd;
      }

protected:
    /*!
      \brief open socket
      \return value of close(fd) if socket is opened.
    */
    bool open( const SocketType type );

    /*!
      \brief bind the socket to local address
      \param port port number
      \return true if successfully binded.
    */
    bool bind( const HostAddress::PortNumber port );

    /*!
      \brief set the address info of the specified remote host.
      \param hostname the name of remote host (or IP address)
      \param port port number of remote host.
      \return true if generated binary address of remote host.
    */
    bool setPeerAddress( const char * hostname,
                         const HostAddress::PortNumber port );

    /*!
      \brief set non blocking mode.
      \return returned value of fcntl()
    */
    int setNonBlocking();

    /*!
      \brief connect to address set by setAddr()
      \return 0 if successful, otherwise -1
    */
    int connectToPresetAddr();

public:

    /*!
      \brief check if socket is opened or not.
      \return true if socket has the valid file descripter.
    */
    bool isOpen() const
      {
          return fd() != -1;
      }

    /*!
      \brief close socket
      \return value that close(fd) returns if socket is opened, otherwise 0.
    */
    int close();

    /*!
      \brief get the local address object
      \return address object
     */
    const HostAddress & localAddress() const
      {
          return M_local_address;
      }

    /*!
      \brief get the port number of destination address
      \return port number
     */
    HostAddress::PortNumber localPort() const
      {
          return M_local_address.portNumber();
      }

    /*!
      \brief get the peer address object
      \return address object
     */
    const HostAddress & peerAddress() const
      {
          return M_peer_address;
      }

    /*!
      \brief get the host name of destination address
      \return host name string
     */
    std::string peerName() const
      {
          return M_peer_address.toHostName();
      }

    /*!
      \brief get the port number of destination address
      \return port number
     */
    HostAddress::PortNumber peerPort() const
      {
          return M_peer_address.portNumber();
      }

};

} // end namespace

#endif
