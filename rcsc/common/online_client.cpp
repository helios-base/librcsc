// -*-c++-*-

/*!
  \file online_client.cpp
  \brief standard soccer client class Source File.
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

#include "online_client.h"

#include "soccer_agent.h"

#include <rcsc/net/udp_socket.h>

#include <iostream>
#include <cstring>

#include <unistd.h> // select()
#include <sys/select.h> // select()
#include <sys/time.h> // select()
#include <sys/types.h> // select()

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
OnlineClient::OnlineClient()
    : AbstractClient()
{

}

/*-------------------------------------------------------------------*/
/*!

 */
OnlineClient::~OnlineClient()
{
    if ( M_offline_out.is_open() )
    {
        M_offline_out.flush();
        M_offline_out.close();
    }
    // std::cerr << "delete OnlineClient" << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
OnlineClient::run( SoccerAgent * agent )
{
    assert( agent );

    if ( ! handleStart( agent )
         || ! isServerAlive() )
    {
        handleExit( agent );
        return;
    }

    // set interval timeout
    struct timeval interval;

    fd_set read_fds;
    fd_set read_fds_back;

    FD_ZERO( &read_fds );
    FD_SET( M_socket->fd(), &read_fds );
    read_fds_back = read_fds;

    int timeout_count = 0;
    int waited_msec = 0;

    while ( isServerAlive() )
    {
        read_fds = read_fds_back;
        interval.tv_sec = intervalMSec() / 1000;
        interval.tv_usec = ( intervalMSec() % 1000 ) * 1000;

        int ret = ::select( M_socket->fd() + 1, &read_fds,
                            static_cast< fd_set * >( 0 ),
                            static_cast< fd_set * >( 0 ),
                            &interval );
        if ( ret < 0 )
        {
            perror( "select" );
            break;
        }
        else if ( ret == 0 )
        {
            // no meesage. timeout.
            waited_msec += intervalMSec();
            ++timeout_count;
            handleTimeout( agent, timeout_count, waited_msec );
        }
        else
        {
            // received message, reset wait time
            waited_msec = 0;
            timeout_count = 0;
            handleMessage( agent );
        }
    }

    handleExit( agent );
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
OnlineClient::connectTo( const char * hostname,
                         const int port )
{
    M_socket = std::shared_ptr< UDPSocket >( new UDPSocket( hostname, port ) );

    if ( ! M_socket
         || M_socket->fd() == -1 )
    {
        std::cerr << "(OnlineClinet::connectTo) Failed to create connection."
                  << std::endl;
        setServerAlive( false );
        return false;
    }

    setServerAlive( true );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
int
OnlineClient::sendMessage( const char * msg )
{
    if ( ! M_socket )
    {
        return 0;
    }

    compress( msg );

    if ( ! M_sent_message.empty() )
    {
        return M_socket->writeDatagram( M_sent_message.data(),
                                        M_sent_message.length() );
    }

    return 0;

    // std::cerr << "send [" << msg << "0]" << endl;
    // if the length of message is the result of only strlen,
    // server will reply "(warning message_not_null_terminated)"
    // return M_socket->send( msg, std::strlen( msg ) + 1 );
}

/*-------------------------------------------------------------------*/
/*!

 */
int
OnlineClient::receiveMessage()
{
    static char msg[MAX_MESG];

    if ( ! M_socket )
    {
        return 0;
    }

    int n = M_socket->readDatagram( msg, MAX_MESG );

    if ( n > 0 )
    {
        decompress( msg, n );

        if ( M_offline_out.is_open() )
        {
            M_offline_out << M_received_message << '\n';
        }
    }

    return n;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
OnlineClient::openOfflineLog( const std::string & filepath )
{
    M_offline_out.close();
    M_offline_out.open( filepath.c_str() );

    if ( ! M_offline_out.is_open() )
    {
        return false;
    }

    if ( ! M_received_message.empty() )
    {
        M_offline_out << M_received_message << std::endl;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
OnlineClient::printOfflineThink()
{
    if ( M_offline_out.is_open() )
    {
        M_offline_out << "(think)" << std::endl;
    }
}

}
