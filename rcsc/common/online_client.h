// -*-c++-*-

/*!
  \file online_client.h
  \brief standard soccer client class Header File.
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

#ifndef RCSC_COMMON_ONLINE_CLIENT_H
#define RCSC_COMMON_ONLINE_CLIENT_H

#include <rcsc/common/abstract_client.h>

#include <fstream>

namespace rcsc {

class UDPSocket;

/*!
  \class OnlineClient
  \brief standard soccer clientt class.

  This class
   - supplies UDP connection interfece to the rcssserver.
   - bridges socket event to the agent.
   - supplies an automated gzip compressor/decompressor.
 */
class OnlineClient
    : public AbstractClient {
private:

    //! udp connection
    std::shared_ptr< UDPSocket > M_socket;

    //! output file for offline logging
    std::ofstream M_offline_out;

public:

    /*!
      \brief default constructor.
     */
    OnlineClient();

    /*!
      \brief destructor.
     */
    ~OnlineClient();

    /*!
      \brief program mainloop
      \param agent pointer to the soccer agent instance.

      Thie method keep infinite loop while client can estimate server is alive.
      To handle server message, select() is used.
      Timeout interval of select() is specified by M_interval_msec member variable.
      When server message is received, handleMessage() is called.
      When timeout occurs, handleTimeout() is called.
      When server is not alive, loop is end and handleExit() is called.
     */
    virtual
    void run( SoccerAgent * agent );

    /*!
      \brief connect to the soccer server with timeout value for select()
      \param hostname server host name
      \param port server port number
      \return true if connection is created.
     */
    virtual
    bool connectTo( const char * hostname,
                    const int port );

    /*!
      \brief send raw string to the server
      \param msg message to be sent
      \return result of ::sendto()
     */
    virtual
    int sendMessage( const char * msg );


    /*!
      \brief receive server message in the socket queue.
      If an offline log file is opened, all received messages are recoreded to the file.
      \return length of received message
     */
    virtual
    int receiveMessage();

    /*!
      \brief open the offline client log file.
      \param filepath file path string to be opened.
      \return result status.
     */
    virtual
    bool openOfflineLog( const std::string & filepath ) ;

    /*!
      \brief write "(think)" message to the offline log file.
     */
    virtual
    void printOfflineThink();

};

}

#endif
