// -*-c++-*-

/*!
  \file clang_manager.cpp
  \brief coach language status manager Source File
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

#include "clang_manager.h"
#include "coach_config.h"

#include <iostream>
#include <cstring>
#include <cstdlib>

const int CLangManager::DEFAULT_CLANG_MIN_VER = 7;
const int CLangManager::DEFAULT_CLANG_MAX_VER = 8;

/*-------------------------------------------------------------------*/
/*!

*/
CLangManager::CLangManager()
    : M_clang_min_ver( DEFAULT_CLANG_MIN_VER )
    , M_clang_max_ver( DEFAULT_CLANG_MAX_VER )
    , M_last_define_time( -1, 0 )
    , M_last_meta_time( -1, 0 )
    , M_last_advice_time( -1, 0 )
    , M_last_info_time( -1, 0 )
    , M_freeform_count( 0 )
{
    for ( int i = 0; i < 11; i++ )
    {
        M_teammate_clang_ver[i].first = DEFAULT_CLANG_MIN_VER;
        M_teammate_clang_ver[i].second = DEFAULT_CLANG_MAX_VER;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CLangManager::parseCLangVer( const char * msg )
{
    // "(clang (ver <PLAYER_SHORT_NAME> <MIN> <MAX>))";
    // <PLAYER_SHORT_NAME>: eg. (p "HELIOS" 1 goalie)
    // "(clang (ver (p "HELIOS" 1 goalie) 7 8))";

    if ( std::strncmp( msg, "(clang (ver", std::strlen("(clang (ver") ) )
    {
        std::cerr << "clang parse error. " << msg << std::endl;
        return;
    }

    const char *tok = msg;

    /////////////////////////////////////////////
    // check player id

    bool our_team = false;

    const size_t name_len = CoachConfig::i().teamName().length();
    ++tok;
    while ( *tok != '\"' ) ++tok; //" skip to team name
    ++tok; // skip double-quatation

    if ( *(tok + name_len) == '\"' //"
         && ! std::strncmp( CoachConfig::i().teamName().c_str(), tok, name_len ) )
    {
        our_team = true;
    }

    while ( *tok != '\"' ) ++tok; //" skip to the end of player name
    ++tok; // skip double-quatation

    char *nextp;
    int num = static_cast< int >( std::strtol( tok, &nextp, 10 ) );
    tok = nextp;

    if ( num < 1 || 11 < num )
    {
        std::cerr << "CLangVer::Unexpected uniform number. " << num
                    << '[' << msg << ']' << std::endl;
        return;
    }

    while ( *tok != ')' ) ++tok; // skip to the end of player name
    ++tok; // skip parenthis

    /////////////////////////////////////////////
    // read versin

    int ver_min = static_cast< int >( std::strtol( tok, &nextp, 10 ) );
    tok = nextp;
    int ver_max = static_cast< int >( std::strtol( tok, NULL, 10 ) );

    if ( ver_min < DEFAULT_CLANG_MIN_VER
         || DEFAULT_CLANG_MAX_VER < ver_max
         || ver_min > ver_max )
    {
        std::cerr << "CLangVer::Unexpected clang ver. "
                    << " min=" << ver_min << " max=" << ver_max
                    << '[' << msg << ']' << std::endl;
        return;
    }

    if ( our_team )
    {
        //cerr << "clang our " << num << " min=" << ver_min << " max=" << ver_max << std::endl;
        if ( M_clang_min_ver < ver_min )
        {
            M_clang_min_ver = ver_min;
        }
        if ( M_clang_max_ver > ver_max )
        {
            M_clang_max_ver = ver_max;
        }
        M_teammate_clang_ver[num - 1].first = ver_min;
        M_teammate_clang_ver[num - 1].second = ver_max;
    }
    /*
    else
    {
        cout << "clang their " << num << " min=" << ver_min << " max=" << ver_max << std::endl;
        M_opponent_clang_ver[num - 1].first = ver_min;
        M_opponent_clang_ver[num - 1].second = ver_max;
    }
    */
}
