// -*-c++-*-

/*!
  \file soccer_agent.h
  \brief abstract soccer agent class Header File.
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

#ifndef RCSC_COMMON_SOCCER_AGENT_H
#define RCSC_COMMON_SOCCER_AGENT_H

#include <memory>
#include <list>
#include <string>

namespace rcsc {

class AbstractClient;
class CmdLineParser;

/*!
  \class SoccerAgent
  \brief abstract soccer agent class

  This class supplies:
  - interface for the command line options
  - interface to handle server message
  - interface to handle timeout event
  - interface to handle exit event
 */
class SoccerAgent {
public:
    friend class AbstractClient;

protected:
    //! interface to the rcssserver or offline log.
    std::shared_ptr< AbstractClient > M_client;

private:

    // nocopyable
    SoccerAgent( const SoccerAgent & );
    SoccerAgent & operator=( const SoccerAgent & );

public:

    /*!
      \brief nothing to do. just set NULL to M_client
     */
    SoccerAgent();

    /*!
      \brief virtual destructor.
     */
    virtual
    ~SoccerAgent();

    /*!
      \brief initialize with command line options.
      \param cmd_parser command line parser instance that contains option strings
      \return initialization result of the derived class.

      Connection must be created after this method.
      You should specify the server host name, port number
      and wait interval msec in initImpl() virtual method in
      the derived class.
      (init) commad is sent in run() method. So, do not call it yourself!
    */
    bool init( CmdLineParser & cmd_parser );

    /*!
      \param client pointer to the client object instance
     */
    void setClient( std::shared_ptr< AbstractClient > client );

    /*!
      \brief create standard console client object (online or offline) according to the command line option.
      \return client object pointer.
     */
    virtual
    std::shared_ptr< AbstractClient > createConsoleClient() = 0;

protected:

    /*!
      \brief init interval status using command line options
      \param cmd_parser command line parser object
      \return derived class should return the status of initialization result.

      This method is called from init(client,argc,argv);
      This method must be overrided in the derived class.
    */
    virtual
    bool initImpl( CmdLineParser & cmd_parser ) = 0;

    /*!
      \brief (pure virtual) handle start event
      \return status of start procedure.

      This method is called at the top of AbstractClient::run() method.
      The concrete agent must connect to the server and send init command.
      Do NOT call this method by yourself!
     */
    virtual
    bool handleStart() = 0;

    /*!
      \brief (virtual) handle start event in offline client mode.
      \return status of start procedure.

      This method is called at the top of AbstractClient::run() method.
      The concrete agent must connect to the server and send init command.
      Do NOT call this method by yourself!
     */
    virtual
    bool handleStartOffline()
      {
          return true;
      }

    /*!
      \brief (pure virtual) handle server messege

      This method is called when server message is arrived.
     */
    virtual
    void handleMessage() = 0;


    /*!
      \brief (virtual) handle offline client log message in offline client mode.

      This method is called when offline client log message is read.
     */
    virtual
    void handleMessageOffline()
      { }

    /*!
      \brief (pure virtual) handle timeout event
      \param timeout_count count of timeout without sensory message.
      \param waited_msec elapsed milli seconds sinc last sensory message.

      This method is called when select() timeout occurs
     */
    virtual
    void handleTimeout( const int timeout_count,
                        const int waited_msec ) = 0;

    /*!
      \brief (pure virtual) handle exit event

      This method is called when client estimates server is not alive.
     */
    virtual
    void handleExit() = 0;
};

}

#endif
