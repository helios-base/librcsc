// -*-c++-*-

/*!
  \file soccer_agent.cpp
  \brief abstract soccer agent class Source File.
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

#include "soccer_agent.h"

#include "abstract_client.h"

#include <rcsc/param/cmd_line_parser.h>
#include <rcsc/version.h>

#include <iostream>
#include <cassert>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
SoccerAgent::SoccerAgent()
    : M_client()
{

}

/*-------------------------------------------------------------------*/
/*!

*/
SoccerAgent::~SoccerAgent()
{
    //std::cerr << "delete SoccerAgent" << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
SoccerAgent::init( CmdLineParser & cmd_parser )
{
   /*
      Do NOT remove the following copyright notice!
     */
    std::cout << copyright() << std::flush;

    if ( ! initImpl( cmd_parser ) )
    {
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
SoccerAgent::setClient( std::shared_ptr< AbstractClient > client )
{
    assert( client );

    M_client = client;
}

}
