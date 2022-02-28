// -*-c++-*-

/*!
  \file abstract_client.h
  \brief abstract soccer client class Header File.
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

#ifndef RCSC_COMMON_ABSTRACT_CLIENT_H
#define RCSC_COMMON_ABSTRACT_CLIENT_H

#include <memory>
#include <string>

namespace rcsc {

class GZCompressor;
class GZDecompressor;
class SoccerAgent;

/*!
  \class AbstractClient
  \brief abstract soccer client class
 */
class AbstractClient {
public:

    enum {
        MAX_MESG = 8192, //!< max length of send/receive buffer.
    };

private:

    //! server status flag.
    bool M_server_alive;

    //! timeout interval for select() or similar timer mechanism.
    int M_interval_msec;

    //! send message compressor
    std::shared_ptr< GZCompressor > M_compressor;

    //! receive message decompressor
    std::shared_ptr< GZDecompressor > M_decompressor;

    //! gzip compression level
    int M_compression_level;

protected:

    //! sent (compressed) message buffer
    std::string M_sent_message;

    //! received (decompressed) message buffer
    std::string M_received_message;

private:

    // nocopyable
    AbstractClient( const AbstractClient & );
    AbstractClient & operator=( const AbstractClient & );

protected:

    /*!
      \brief default constructor (protected).
     */
    AbstractClient();

public:

    /*!
      \brief virtual destructor.
     */
    virtual
    ~AbstractClient();

    /*!
      \brief program mainloop. This method will be used only by console client.
      In the case of GUI client development, you need to use the mechanism prepared by GUI library.
      \param agent pointer to the soccer agent instance.
     */
    virtual
    void run( SoccerAgent * agent ) = 0;

    /*!
      \brief connect to the soccer server with timeout value for select()
      \param hostname server host name
      \param port server port number
      \return true if connection is created.
     */
    virtual
    bool connectTo( const char * hostname,
                    const int port ) = 0;

    /*!
      \brief send raw string to the server
      \param msg message to be sent
      \return result of ::sendto()
     */
    virtual
    int sendMessage( const char * msg ) = 0;


    /*!
      \brief receive server message in the socket queue
      \return length of received message
     */
    virtual
    int receiveMessage() = 0;

    /*!
      \brief open the offline client log file.
      \param filepath file path string to be opened.
      \return result status.
     */
    virtual
    bool openOfflineLog( const std::string & filepath ) = 0;

    /*!
      \brief write "(think)" message to the offline log file.
     */
    virtual
    void printOfflineThink() = 0;

    /*!
      \brief set new interval time for select()
      \param interval_msec new interval by milli second
     */
    void setIntervalMSec( const int interval_msec );

    /*!
      \brief get the current timeout interval.
      \return interval time in milli-second.
     */
    int intervalMSec() const
      {
          return M_interval_msec;
      }

    /*!
      \brief set server status
      \param alive server status flag. if server is dead, this value becomes false.
     */
    void setServerAlive( const bool alive )
      {
          M_server_alive = alive;
      }

    /*!
      \brief check server alive status
      \return true if client can estimate server is alive
     */
    bool isServerAlive() const
      {
          return M_server_alive;
      }

    /*!
      \brief set gzip compression level
      \param level compression level
      \return old compression level
     */
    int setCompressionLevel( const int level );

    /*!
      \brief get the current gzip compression level.
      \return gzip compression level [0-9].
     */
    int compressionLevel() const
      {
          return M_compression_level;
      }

    /*!
      \brief get the last recieved message.
      \return const pointer to the message buffer.
     */
    const char * message() const
      {
          return M_received_message.c_str();
      }

protected:

    /*!
      \brief compress the outgoing message. the comppressed message is stored in M_sent_message.
      \param msg raw command message string. the massage has to be null-terminated.
     */
    void compress( const char * msg );

    /*!
      \brief decompress the received message. the decomppressed message is stored in M_received_message.
      \param msg raw received message string. the message may not be null-terminated.
      \param n the length of received message.
     */
    void decompress( const char * msg,
                     const int n );

    /*!
      \brief just call agent->handleStart()
      \param agent pointer to the agent instance.
      \return result status.
     */
    bool handleStart( SoccerAgent * agent );

    /*!
      \brief just call agent->handleStartOffline()
      \param agent pointer to the agent instance.
      \return result status.
     */
    bool handleStartOffline( SoccerAgent * agent );

    /*!
      \brief just call agent->handleMessage()
      \param agent pointer to the agent instance.
     */
    void handleMessage( SoccerAgent * agent );

    /*!
      \brief just call agent->handleMessageOffline()
      \param agent pointer to the agent instance.
     */
    void handleMessageOffline( SoccerAgent * agent );

    /*!
      \brief just call agent->handleTimeout()
      \param agent pointer to the agent instance.
     */
    void handleTimeout( SoccerAgent * agent,
                        const int timeout_count,
                        const int waited_msec );

    /*!
      \brief just call agent->handleExit()
      \param agent pointer to the agent instance.
     */
    void handleExit( SoccerAgent * agent );

};

}

#endif
