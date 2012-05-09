// -*-c++-*-

/*!
  \file result_writer.cpp
  \brief concrete rcg data handler Source File.
*/

/*
 *Copyright:

 Copyright (C) 2004 Hidehisa Akiyama

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
 any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#include "result_writer.h"

#include <rcsc/rcg/util.h>

#include <cstring>
#include <iostream>

/*-------------------------------------------------------------------*/
/*!

*/
ResultWriter::ResultWriter()
    : M_left_team_name("")
    , M_right_team_name("")
    , M_left_score( 0 )
    , M_right_score( 0 )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ResultWriter::handleTeamInfo( const rcsc::rcg::team_t& team_left,
                              const rcsc::rcg::team_t& team_right )
{
    if ( M_left_team_name.empty() )
    {
        char buf[18];
        std::memset( buf, '\0', 18 );
        std::strncpy( buf, team_left.name, 16 );
        M_left_team_name = buf;
    }
    if ( M_right_team_name.empty() )
    {
        char buf[18];
        std::memset( buf, '\0', 18 );
        std::strncpy( buf, team_right.name, 16 );
        M_right_team_name = buf;
    }

    M_left_score = rcsc::rcg::nstohi( team_left.score );
    M_right_score = rcsc::rcg::nstohi( team_right.score );

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ResultWriter::handleEOF()
{
    std::cout << M_left_team_name << ": " << M_left_score
              << " - "
              << M_right_score << " :" << M_right_team_name
              << std::endl;
    return true;
}
