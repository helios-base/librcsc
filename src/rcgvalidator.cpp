// -*-c++-*-

/*!
  \file rcgverconv.cpp
  \brief rcg version converter source File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa Akiyama

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

#include <rcsc/common/server_param.h>
#include <rcsc/common/player_param.h>
#include <rcsc/common/player_type.h>

#include <rcsc/types.h>
#include <rcsc/gz.h>
#include <rcsc/rcg.h>

using namespace rcsc;
using namespace rcsc::rcg;

/*-------------------------------------------------------------------*/
/*!

 */
class Validator
    : public Handler {
private:

    PlayMode M_last_playmode;
    int M_last_game_time;
    int M_player_missing_count;

public:
    Validator();

    bool handleLogVersion( const int ver ) override;
    bool handleEOF() override;

    bool handleShow( const ShowInfoT & show ) override;

    bool handleMsg( const int /*time*/,
                    const int /*board*/,
                    const std::string & /*msg*/ ) override
    {
        return true;
    }
    bool handleDraw( const int /*time*/,
                     const drawinfo_t & /*draw*/ ) override
    {
        return true;
    }

    bool handlePlayMode( const int time,
                         const PlayMode pm ) override;

    bool handleTeam( const int /*time*/,
                     const TeamT & /*team_l*/,
                     const TeamT & /*team_r*/ ) override
    {
        return true;
    }

    bool handleServerParam( const ServerParamT & param ) override;

    bool handlePlayerParam( const PlayerParamT & /*param*/ ) override
    {
        return true;
    }
    bool handlePlayerType( const PlayerTypeT & /*param*/ ) override
    {
        return true;
    }
    bool handleTeamGraphic( const char /*side*/,
                            const int /*x*/,
                            const int /*y*/,
                            const std::vector< std::string > & /*xpm_data*/ ) override
    {
        return true;
    }

};


/*-------------------------------------------------------------------*/
Validator::Validator()
    : rcg::Handler(),
      M_last_game_time( 0 ),
      M_player_missing_count( 0 )
{

}

/*-------------------------------------------------------------------*/
bool
Validator::handleLogVersion( const int ver )
{
    rcg::Handler::handleLogVersion( ver );

    if ( ver < 4 )
    {
        std::cerr << "Unsupported RCG version " << ver << std::endl;
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
Validator::handleEOF()
{
    const int assumed_game_count
        = ServerParam::i().actualHalfTime()
        * ServerParam::i().nrNormalHalfs();

    if ( M_last_game_time < assumed_game_count - 1 )
    {
        std::cerr << "(rcgvalidator) [false] "
                  << "last game time: " << M_last_game_time
                  << " << assumed count: " << assumed_game_count
                  << std::endl;
        return false;
    }

    if ( M_player_missing_count >= 10 )
    {
        std::cerr << "(rcgvalidator) [false] missing player count = "
                  << M_player_missing_count << std::endl;
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
Validator::handleShow( const ShowInfoT & show )
{
    if ( M_last_playmode != PM_BeforeKickOff
         && M_last_playmode != PM_TimeOver )
    {
        for ( int i = 0; i < rcsc::MAX_PLAYER*2; ++i )
        {
            if ( ! show.player_[i].isAlive()
                 && ! show.player_[i].hasRedCard() )
            {
                M_player_missing_count += 1;
                // std::cerr << "(rcgvalidator) detected missing player at time=["
                //           << show.time_ << ',' << show.stime_ << ']'
                //           << std::endl;
                break;
            }
        }
    }

    M_last_game_time = show.time_;
    return true;
}

/*-------------------------------------------------------------------*/
bool
Validator::handlePlayMode( const int time,
                           const PlayMode pm )
{
    M_last_playmode = pm;
    M_last_game_time = time;
    return true;
}

/*-------------------------------------------------------------------*/
bool
Validator::handleServerParam( const ServerParamT & param )
{
    ServerParam::instance().convertFrom( param );
    return true;
}


/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/

int
main( int argc, char** argv )
{
    if ( argc < 2
         || ! strcmp( argv[1], "--help" )
         || ! strcmp( argv[1], "-h" ) )
    {
        std::cerr << "usage: " << argv[0]
                  << " <RcgFile>[.gz]"
                  << " [outputFile]"
                  << std::endl;
        return 0;
    }

    const std::string infile = argv[1];

    rcsc::gzifstream fin( infile.c_str() );

    if ( ! fin.is_open() )
    {
        std::cerr << "Could not open the input file : " << infile << std::endl;
        return 1;
    }

    Parser::Ptr parser = rcsc::rcg::Parser::create( fin );

    if ( ! parser )
    {
        std::cerr << "Could not create the rcg parser." << std::endl;
        return 1;
    }

    Validator validator;
    if ( ! parser->parse( fin, validator ) )
    {
        return 1;
    }

    return 0;
}
