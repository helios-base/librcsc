// -*-c++-*-

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <rcsc/gz.h>
#include <rcsc/rcg.h>

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cmath>
#include <cstring>

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

class Reverser
    : public rcsc::rcg::Handler {
private:

    std::ostream & M_os;

    rcsc::rcg::Serializer::Ptr M_serializer;

    // not used
    Reverser() = delete;
public:

    explicit
    Reverser( std::ostream & os );

    bool handleLogVersion( const int ver );

    bool handleEOF();

    bool handleShow( const rcsc::rcg::ShowInfoT & show );
    bool handleMsg( const int time,
                    const int board,
                    const std::string & msg );
    bool handleDraw( const int ,
                     const rcsc::rcg::drawinfo_t & )
      {
          return true;
      }
    bool handlePlayMode( const int time,
                         const rcsc::PlayMode pm );
    bool handleTeam( const int time,
                     const rcsc::rcg::TeamT & team_l,
                     const rcsc::rcg::TeamT & team_r );
    bool handleServerParam( const std::string & msg );
    bool handlePlayerParam( const std::string & msg );
    bool handlePlayerType( const std::string & msg );

private:

    rcsc::rcg::BallT reverse( const rcsc::rcg::BallT & ball );
    rcsc::rcg::PlayerT reverse( const rcsc::rcg::PlayerT & player );
};


/*-------------------------------------------------------------------*/
/*!

*/
Reverser::Reverser( std::ostream & os )
    : M_os( os )
{
    M_serializer = rcsc::rcg::Serializer::create( 1 );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Reverser::handleLogVersion( const int ver )
{
    rcsc::rcg::Handler::handleLogVersion( ver );

    M_serializer = rcsc::rcg::Serializer::create( ver );

    if ( ! M_serializer )
    {
        return false;
    }

    M_serializer->serializeHeader( M_os );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Reverser::handleEOF()
{
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Reverser::handleShow( const rcsc::rcg::ShowInfoT & show )
{
    if ( ! M_serializer )
    {
        return false;
    }

    rcsc::rcg::ShowInfoT new_show;

    new_show.ball_ = reverse( show.ball_ );

    for ( int i = 0; i < rcsc::MAX_PLAYER*2; ++i )
    {
        int idx = show.player_[i].unum_ - 1;
        if ( show.player_[i].side_ == 'l' ) idx += rcsc::MAX_PLAYER;
        if ( idx < 0 || rcsc::MAX_PLAYER*2 <= idx ) continue;

        new_show.player_[idx] = reverse( show.player_[i] );
    }

    new_show.time_ = show.time_;

    M_serializer->serialize( M_os, new_show );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Reverser::handleMsg( const int,
                     const int board,
                     const std::string & msg )
{
    if ( ! M_serializer )
    {
        return false;
    }

    M_serializer->serialize( M_os, board, msg );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Reverser::handlePlayMode( const int,
                          const rcsc::PlayMode pm )
{
    if ( ! M_serializer )
    {
        return false;
    }

    M_serializer->serialize( M_os, pm );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Reverser::handleTeam( const int,
                      const rcsc::rcg::TeamT & team_l,
                      const rcsc::rcg::TeamT & team_r )
{
    if ( ! M_serializer )
    {
        return false;
    }

    M_serializer->serialize( M_os, team_r, team_l );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Reverser::handleServerParam( const std::string & msg )
{
    if ( ! M_serializer )
    {
        return false;
    }

    M_serializer->serializeParam( M_os, msg );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Reverser::handlePlayerParam( const std::string & msg )
{
    if ( ! M_serializer )
    {
        return false;
    }

    M_serializer->serializeParam( M_os, msg );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Reverser::handlePlayerType( const std::string & msg )
{
    if ( ! M_serializer )
    {
        return false;
    }

    M_serializer->serializeParam( M_os, msg );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
rcsc::rcg::BallT
Reverser::reverse( const rcsc::rcg::BallT & ball )
{
    rcsc::rcg::BallT new_ball;

    new_ball.x_ = - ball.x_;
    new_ball.y_ = - ball.y_;
    new_ball.vx_ = - ball.vx_;
    new_ball.vy_ = - ball.vy_;

    return new_ball;
}

/*-------------------------------------------------------------------*/
/*!

*/
rcsc::rcg::PlayerT
Reverser::reverse( const rcsc::rcg::PlayerT & player )
{
    rcsc::rcg::PlayerT new_player = player;

    new_player.side_ = ( player.side_ == 'l' ? 'r' : 'l' );

    if ( player.state_ != rcsc::rcg::DISABLE )
    {
        new_player.x_ = - player.x_;
        new_player.y_ = - player.y_;
        new_player.vx_ = - player.vx_;
        new_player.vy_ = - player.vy_;
        new_player.body_ += 180.0;
        if ( new_player.body_ > 180.0 ) new_player.body_ -= 360.0;
    }
    else
    {
        new_player.x_ = - player.x_;
    }

    return new_player;
}

////////////////////////////////////////////////////////////////////////

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

    rcsc::gzifstream fin( argv[1] );

    if ( ! fin.is_open() )
    {
        std::cerr << "Failed to open the input file : " << argv[1] << std::endl;
        return 1;
    }

    std::string out_filepath;
    if ( argc >= 3 )
    {
        out_filepath = argv[2];
    }
    else
    {
        out_filepath = "reverse-";
        out_filepath += argv[1];
    }

    if ( out_filepath.length() > 3
         && out_filepath.compare( out_filepath.length() - 3, 3, ".gz" ) == 0 )
    {
        out_filepath.erase( out_filepath.length() - 3 );
    }

    std::ofstream fout( out_filepath.c_str(), std::ios_base::binary );
    if ( ! fout.is_open() )
    {
        std::cerr << "Failed to open the output file : " << out_filepath << std::endl;
        return 1;
    }

    rcsc::rcg::Parser::Ptr parser = rcsc::rcg::Parser::create( fin );

    if ( ! parser )
    {
        std::cerr << "Failed to create rcg parser." << std::endl;
        return 1;
    }

    std::cout << "input file = " << argv[1] << std::endl;
    std::cout << "output file = " << out_filepath << std::endl;

    // create rcg handler instance
    Reverser reverser( fout );

    parser->parse( fin, reverser );

    return 0;
}
