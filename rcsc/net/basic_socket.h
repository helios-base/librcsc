// -*-c++-*-

/*!
  \file basic_socket.h
  \brief socket class Header File.
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

#ifndef RCSC_NET_BASIC_SOCKET_H
#define RCSC_NET_BASIC_SOCKET_H

#include <boost/scoped_ptr.hpp>
#include <cstddef>

namespace rcsc {

struct AddrImpl;

/*!
  \class BasicSocket
  \brief socket class
*/
class BasicSocket {
private:
    //! socket file descriptor
    int M_fd;

protected:
    //! destination address
    boost::scoped_ptr< AddrImpl > M_dest;

public:
    /*!
      \enum SocketType
      \brief socket type definition
     */
    enum SocketType {
        DATAGRAM_TYPE,
        STREAM_TYPE,
    };

public:
    /*!
      \brief destructor. close socket automatically
    */
    virtual
    ~BasicSocket();

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
      \brief constructor for server socket
    */
    BasicSocket();

    /*!
      \brief set distination port
      \param addr address implementation dependent address
    */
    void setDestPort( const void * addr );


    /*!
      \brief open socket
      \return value of close(fd) if socket is opened.
    */
    bool open( const SocketType type );

    /*!
      \brief bind the socket to local address
      \param port port number to be binded.
      \return true if successfully binded.
    */
    bool bind( const int port = 0 );

    /*!
      \brief set the address info of the specified remote host.
      \param hostname the name of remote host.(or IP address)
      \param port port number of remote host.
      \return true if generated binary address of remote host.
    */
    bool setAddr( const char * hostname,
                  const int port );

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
      \param len maximam length of receive buffer
      \retval 0 error occured and errno is EWOULDBLOCK
      \retval -1 error occured
      \return the length of received data.
    */
    int readFromStream( char * buf,
                        const std::size_t len );

    /*!
      \brief send datagram data to the connected host.
      \param data the pointer to the data to be sent.
      \param len the length of data.
      \return the length of sent data if successfuly sent, otherwise -1.
    */
    int sendDatagramPacket( const char * data,
                            const std::size_t len );

    /*!
      \brief receive datagram data from the connected remote host.
      \param buf buffer to receive data
      \param len maximal length of buffer buf
      \param overwrite_dist_addr if this value is true,
      set distination address to sender address of this packet.
      \retval 0 error occured and errno is EWOULDBLOCK
      \retval -1 error occured
      \return the length of received data.
    */
    int receiveDatagramPacket( char * buf,
                               const std::size_t len,
                               const bool overwrite_dist_addr = false );

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
      \brief send stream data to the connected host.
      \param data the pointer to the data to be sent.
      \param len the length of data.
      \return the length of sent data if successfuly sent, otherwise -1.
     */
    virtual
    int send( const char * data,
              const std::size_t len ) = 0;

    /*!
      \brief receive stream data from the connected remote host.
      \param buf buffer to receive data
      \param len maximal length of buffer buf
      \retval 0 error occured and errno is EWOULDBLOCK
      \retval -1 error occured
      \return the length of received data.
     */
    virtual
    int receive( char * buf,
                 const std::size_t len ) = 0;
};

} // end namespace

#endif
