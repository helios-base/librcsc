// -*-c++-*-

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cmath>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>

#include <rcsc/common/server_param.h>
#include <rcsc/common/player_param.h>
#include <rcsc/common/player_type.h>

#include <rcsc/types.h>
#include <rcsc/gz.h>
#include <rcsc/rcg.h>

class CSVPrinter
    : public rcsc::rcg::Handler {
private:
    struct CommandCount {
        int kick_;
        int dash_;
        int turn_;
        int say_;
        int turn_neck_;
        int catch_;
        int move_;
        int change_view_;

        CommandCount()
            : kick_( 0 )
            , dash_( 0 )
            , turn_( 0 )
            , say_( 0 )
            , turn_neck_( 0 )
            , catch_( 0 )
            , move_( 0 )
            , change_view_( 0 )
          { }
        void update( const rcsc::rcg::player_t & player )
          {
              kick_ = rcsc::rcg::nstohi( player.kick_count );
              dash_ = rcsc::rcg::nstohi( player.dash_count );
              turn_ = rcsc::rcg::nstohi( player.turn_count );
              say_ = rcsc::rcg::nstohi( player.say_count );
              turn_neck_ = rcsc::rcg::nstohi( player.turn_neck_count );
              catch_ = rcsc::rcg::nstohi( player.catch_count );
              move_ = rcsc::rcg::nstohi( player.move_count );
              change_view_ = rcsc::rcg::nstohi( player.change_view_count );
          }
        void update( const rcsc::rcg::PlayerT & player )
          {
              kick_ = player.kick_count_;
              dash_ = player.dash_count_;
              turn_ = player.turn_count_;
              say_ = player.say_count_;
              turn_neck_ = player.turn_neck_count_;
              catch_ = player.catch_count_;
              move_ = player.move_count_;
              change_view_ = player.change_view_count_;
          }
    };


    std::ostream & M_os;

    int M_show_count;

    rcsc::rcg::UInt32 M_cycle;
    rcsc::rcg::UInt32 M_stopped;

    rcsc::PlayMode M_playmode;
    rcsc::rcg::TeamT M_teams[2];

    CommandCount M_command_count[rcsc::MAX_PLAYER * 2];

    // not used
    CSVPrinter() = delete;
public:

    explicit
    CSVPrinter( std::ostream & os );

    virtual
    bool handleLogVersion( const int ver );

    virtual
    bool handleEOF();

    virtual
    bool handleShow( const rcsc::rcg::ShowInfoT & show );
    virtual
    bool handleMsg( const int time,
                    const int board,
                    const std::string & msg );
    virtual
    bool handleDraw( const int time,
                     const rcsc::rcg::drawinfo_t & draw );
    virtual
    bool handlePlayMode( const int time,
                         const rcsc::PlayMode pm );
    virtual
    bool handleTeam( const int time,
                     const rcsc::rcg::TeamT & team_l,
                     const rcsc::rcg::TeamT & team_r );
    virtual
    bool handleServerParam( const std::string & msg );
    virtual
    bool handlePlayerParam( const std::string & msg );
    virtual
    bool handlePlayerType( const std::string & msg );

private:
    const std::string & getPlayModeString( const rcsc::PlayMode playmode ) const;

    std::ostream & printServerParam() const;
    std::ostream & printPlayerParam() const;
    std::ostream & printPlayerTypes() const;


    std::ostream & printShowHeader() const;
    std::ostream & printShowData( const rcsc::rcg::ShowInfoT & show ) const;

    // print values
    std::ostream & printShowCount() const;
    std::ostream & printTime() const;
    std::ostream & printPlayMode() const;
    std::ostream & printTeams() const;
    std::ostream & printBall( const rcsc::rcg::BallT & ball ) const;
    std::ostream & printPlayers( const rcsc::rcg::ShowInfoT & show ) const;
    std::ostream & printPlayer( const rcsc::rcg::PlayerT & player ) const;

};


/*-------------------------------------------------------------------*/
/*!

 */
CSVPrinter::CSVPrinter( std::ostream & os )
    : M_os( os ),
      M_show_count( 0 ),
      M_cycle( 0 ),
      M_stopped( 0 ),
      M_playmode( rcsc::PM_Null )
{

}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CSVPrinter::handleLogVersion( const int ver )
{
    rcsc::rcg::Handler::handleLogVersion( ver );

    if ( ver < 4 )
    {
        std::cerr << "Unsupported RCG version " << ver << std::endl;
        return false;
    }

    return true;
}


/*-------------------------------------------------------------------*/
/*!

 */
bool
CSVPrinter::handleEOF()
{
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CSVPrinter::handleShow( const rcsc::rcg::ShowInfoT & show )
{
    static bool s_first = true;
    if ( s_first )
    {
        printShowHeader();
        s_first = false;
    }

    // update show count
    ++M_show_count;

    // update game time
    if ( M_cycle == show.time_ )
    {
        ++M_stopped;
    }
    else
    {
        M_cycle = show.time_;
        M_stopped = 0;
    }

    printShowData( show );

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CSVPrinter::handleMsg( const int,
                       const int,
                       const std::string & )
{
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CSVPrinter::handleDraw( const int,
                        const rcsc::rcg::drawinfo_t & )
{
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CSVPrinter::handlePlayMode( const int,
                            const rcsc::PlayMode pm )
{
    M_playmode = pm;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CSVPrinter::handleTeam( const int,
                        const rcsc::rcg::TeamT & team_l,
                        const rcsc::rcg::TeamT & team_r )
{
    M_teams[0] = team_l;
    M_teams[1] = team_r;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CSVPrinter::handleServerParam( const std::string & msg )
{
    if ( ! rcsc::ServerParam::instance().parse( msg.c_str(), 8 ) )
    {
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CSVPrinter::handlePlayerParam( const std::string & msg )
{
    if ( ! rcsc::PlayerParam::instance().parse( msg.c_str(), 8 ) )
    {
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CSVPrinter::handlePlayerType( const std::string & )
{
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
const std::string &
CSVPrinter::getPlayModeString( const rcsc::PlayMode playmode ) const
{
    static const std::string s_playmode_str[] = PLAYMODE_STRINGS;

    if ( playmode < rcsc::PM_Null
         || rcsc::PM_MAX < playmode )
    {
        return s_playmode_str[0];
    }

    return s_playmode_str[playmode];
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CSVPrinter::printServerParam() const
{
    return M_os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CSVPrinter::printPlayerParam() const
{
    return M_os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CSVPrinter::printPlayerTypes() const
{
    return M_os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CSVPrinter::printShowHeader() const
{
    M_os << "#"
         << ", cycle, stopped"
         << ", playmode"
         << ", l_name, l_score, l_pen_score"
         << ", r_name, r_score, r_pen_score"
         << ", b_x, b_y, b_vx, b_vy";

    char side = 'l';
    for ( int s = 0; s < 2; ++s )
    {
        for ( int i = 1; i <= rcsc::MAX_PLAYER; ++i )
        {
            M_os << ", " << side << i << "_t"
                 << ", " << side << i << "_x"
                 << ", " << side << i << "_y"
                 << ", " << side << i << "_vx"
                 << ", " << side << i << "_vy"
                 << ", " << side << i << "_body"
                 << ", " << side << i << "_neck";
        }
        side = 'r';
    }

    M_os << '\n';
    return M_os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CSVPrinter::printShowData( const rcsc::rcg::ShowInfoT & show ) const
{
    printShowCount();
    printTime();
    printPlayMode();
    printTeams();
    printBall( show.ball_ );
    printPlayers( show );

    M_os << '\n';
    return M_os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CSVPrinter::printShowCount() const
{
    M_os << M_show_count;
    return M_os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CSVPrinter::printTime() const
{
    M_os << ',' << M_cycle << ',' << M_stopped;
    return M_os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CSVPrinter::printPlayMode() const
{
    M_os << ',' << getPlayModeString( M_playmode );
    return M_os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CSVPrinter::printTeams() const
{
    for ( const auto & t : M_teams )
    {
        M_os << ',' << t.name_
             << ',' << t.score_
             << ',' << t.pen_score_;
    }

    return M_os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CSVPrinter::printBall( const rcsc::rcg::BallT & ball ) const
{
    M_os << ',' << ball.x_
         << ',' << ball.y_
         << ',' << ball.vx_
         << ',' << ball.vy_;
    return M_os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CSVPrinter::printPlayers( const rcsc::rcg::ShowInfoT & show ) const
{
    for ( const auto & p : show.player_ )
    {
        printPlayer( p );
    }

    return M_os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CSVPrinter::printPlayer( const rcsc::rcg::PlayerT & player ) const
{
    if ( player.state_ == rcsc::rcg::DISABLE )
    {
        M_os << ',' //<< player.type_
             << ',' //<< player.x_
             << ',' //<< player.y_
             << ',' //<< player.vx_
             << ',' //<< player.vy_
             << ',' //<< player.body_
             << ',' //<< player.neck_;
            ;
    }
    else
    {
        M_os << ',' << player.type_
             << ',' << player.x_
             << ',' << player.y_
             << ',' << player.vx_
             << ',' << player.vy_
             << ',' << player.body_
             << ',' << player.neck_
            ;
    }
    return M_os;
}

////////////////////////////////////////////////////////////////////////

int
main( int argc, char** argv )
{
    if ( argc != 2
         || ! std::strncmp( argv[1], "--help", 6 )
         || ! std::strncmp( argv[1], "-h", 2 ) )
    {
        std::cerr << "usage: " << argv[0] << " <RCGFile>[.gz]" << std::endl;
        return 0;
    }

    rcsc::gzifstream fin( argv[1] );

    if ( ! fin.is_open() )
    {
        std::cerr << "Failed to open file : " << argv[1] << std::endl;
        return 1;
    }

    rcsc::rcg::Parser::Ptr parser = rcsc::rcg::Parser::create( fin );

    if ( ! parser )
    {
        std::cerr << "Failed to create rcg parser." << std::endl;
        return 1;
    }

    CSVPrinter printer( std::cout );

    parser->parse( fin, printer );

    return 0;
}
