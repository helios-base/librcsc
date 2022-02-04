// -*-c++-*-

/*!
  \file basic_client.cpp
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

#include "offline_client.h"

#include "soccer_agent.h"

#include <iostream>
#include <cstring>
#include <cassert>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
OfflineClient::OfflineClient()
    : AbstractClient()
{

}

/*-------------------------------------------------------------------*/
/*!

*/
OfflineClient::~OfflineClient()
{
    // std::cerr << "delete OfflineClient" << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
OfflineClient::run( SoccerAgent * agent )
{
    assert( agent );

    if ( ! handleStartOffline( agent ) )
    {
        handleExit( agent );
        return;
    }

    while ( isServerAlive() )
    {
        handleMessageOffline( agent );
    }

    handleExit( agent );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
OfflineClient::connectTo( const char * /*hostname*/,
                          const int /*port*/ )
{
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
int
OfflineClient::sendMessage( const char * /*msg*/ )
{
    return 1;
}

/*-------------------------------------------------------------------*/
/*!

*/
int
OfflineClient::receiveMessage()
{
    while ( std::getline( M_offline_in, M_received_message ) )
    {
        if ( M_received_message.empty() )
        {
            continue;
        }

        return M_received_message.size();
    }

    setServerAlive( false );
    return 0;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
OfflineClient::openOfflineLog( const std::string & filepath )
{
    M_offline_in.close();
    M_offline_in.open( filepath.c_str() );

    return M_offline_in.is_open();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
OfflineClient::printOfflineThink()
{

}

}
