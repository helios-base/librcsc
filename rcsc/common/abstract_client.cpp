// -*-c++-*-

/*!
  \file abstract_client.cpp
  \brief abstract soccer client class Source File.
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

#include "abstract_client.h"

#include "soccer_agent.h"

#include <rcsc/gz/gzcompressor.h>

#include <iostream>
#include <cstring>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
AbstractClient::AbstractClient()
    : M_server_alive( false ),
      M_interval_msec( 10 ),
      M_compression_level( 0 )
{
    M_sent_message.reserve( MAX_MESG );
    M_received_message.reserve( MAX_MESG );
}

/*-------------------------------------------------------------------*/
/*!

*/
AbstractClient::~AbstractClient()
{
    // std::cerr << "delete AbstractClient" << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AbstractClient::setIntervalMSec( const int interval_msec )
{
    if ( interval_msec <= 0 )
    {
        std::cerr << "(AbstractClient::setIntervalMsec) ***ERROR***"
                  << " interval msec have to be positive value. ["
                  << interval_msec << "]" << std::endl;
        return;
    }

    if ( interval_msec < 10 )
    {
        std::cerr << "(AbstractClient::setIntervalMsec) ***ERROR***"
                  << " interval msec have to be more than or equal 10. ["
                  << interval_msec << "]"
                  << std::endl;
        return;
    }

    M_interval_msec = interval_msec;
}

/*-------------------------------------------------------------------*/
/*!

*/
int
AbstractClient::setCompressionLevel( const int level )
{
#ifdef HAVE_LIBZ
    if ( level < 0 || 9 <= level )
    {
        std::cerr << "(AbstractClient::setIntervalMsec) ***ERROR***"
                  << " unsupported compression level "
                  << level << std::endl;
        return M_compression_level;
    }


    int old_level = M_compression_level;
    M_compression_level = level;

    if ( level == 0 )
    {
        M_compressor.reset();
        M_decompressor.reset();
        return old_level;
    }

    M_compressor = std::shared_ptr< GZCompressor >( new GZCompressor( level ) );

    M_decompressor = std::shared_ptr< GZDecompressor >( new GZDecompressor() );

    return old_level;
#else
    (void)level;
    return 0;
#endif
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AbstractClient::compress( const char * msg )
{
#ifdef HAVE_LIBZ
    if ( M_compression_level > 0
         && M_compressor )
    {
        M_compressor->compress( msg,
                                std::strlen( msg ) + 1,
                                M_sent_message );
    }
    else
#endif
    {
        M_sent_message = msg;
        M_sent_message += '\0'; // Non-compressed outgoing messages have to be null-terminated.
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AbstractClient::decompress( const char * msg,
                            const int n )
{
    if ( n <= 0 )
    {
        M_received_message.clear();
        return;
    }

#ifdef HAVE_LIBZ
    if ( M_compression_level > 0
         && M_decompressor )
    {
        M_decompressor->decompress( msg, n, M_received_message );
    }
    else
#endif
    {
        if ( msg[n-1] == '\0' )
        {
            M_received_message.assign( msg, n - 1 );
        }
        else
        {
            M_received_message.assign( msg, n );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
AbstractClient::handleStart( SoccerAgent * agent )
{
    return agent->handleStart();
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
AbstractClient::handleStartOffline( SoccerAgent * agent )
{
    return agent->handleStartOffline();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AbstractClient::handleMessage( SoccerAgent * agent )
{
    agent->handleMessage();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AbstractClient::handleMessageOffline( SoccerAgent * agent )
{
    agent->handleMessageOffline();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AbstractClient::handleTimeout( SoccerAgent * agent,
                               const int timeout_count,
                               const int waited_msec )
{
    agent->handleTimeout( timeout_count, waited_msec );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AbstractClient::handleExit( SoccerAgent * agent )
{
    agent->handleExit();
}

}
