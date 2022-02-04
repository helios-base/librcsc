// -*-c++-*-

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include <cmath>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>

#include <rcsc/gz.h>
#include <rcsc/rcg.h>

/*

// first line
(Init (goal_width 14.02) (player_size 0.299988)
(ball_size 0.0849915) (kickable_margin 0.699997)
(visible_distance 3) (kickable_area 1.08498)
(catchable_area_l 2) (catchable_area_w 1)
(half_time 3000) (ckick_margin 1)
(offside_active_area_size 2.5)
(offside_kick_margin 9.14999)
(audio_cut_dist 50))

(Info (state <time> <playmode> <score_l> <score_r>)
(ball <x> <y> <vx> <vy>)
(player {l|r} <unum>[ g]
(position <x> <y>
<vx> <vy>
<body>   // global body angle (degree)
<head>   // global head angle (degree)
)
(stamina <sta>)
{ (kick[ fault]) | (dash) | (turn) |
(catch[ fault]) | (move) | (tackle[ fault]) |
(none) }
[(say)]
[(turn_neck)]
[(change_view)]
)
)

// last line
(Result "<teamname_l>" "<teamname_r>" <score_l> <score_r>)

*/


class TextPrinter
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

    bool M_init_written;
    rcsc::PlayMode M_playmode;
    std::string M_left_team_name;
    std::string M_right_team_name;
    int M_left_score;
    int M_right_score;

    CommandCount M_command_count[rcsc::MAX_PLAYER * 2];

    // not used
    TextPrinter() = delete;
public:

    explicit
    TextPrinter( std::ostream & os );

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
    const
    std::string & getPlayModeString( const rcsc::PlayMode playmode ) const;


    std::ostream & printState( std::ostream & os,
                               const long & cycle ) const;

    std::ostream & printBall( std::ostream & os,
                              const rcsc::rcg::pos_t & ball ) const;
    std::ostream & printBall( std::ostream & os,
                              const rcsc::rcg::ball_t & ball ) const;

    std::ostream & printPlayer( std::ostream & os,
                                //const std::string & teamname,
                                const rcsc::SideID side,
                                const int unum,
                                const rcsc::rcg::pos_t & player ) const;
    std::ostream & printPlayer( std::ostream & os,
                                //const std::string & teamname,
                                const rcsc::SideID side,
                                const int unum,
                                const CommandCount & count,
                                const rcsc::rcg::player_t & player ) const;
};


/*-------------------------------------------------------------------*/
/*!

 */
TextPrinter::TextPrinter( std::ostream & os )
    : M_os( os ),
      M_init_written( false ),
      M_playmode( rcsc::PM_Null ),
      M_left_team_name( "" ),
      M_right_team_name( "" ),
      M_left_score( 0 ),
      M_right_score( 0 )
{

}

/*-------------------------------------------------------------------*/
/*!

 */
bool
TextPrinter::handleEOF()
{
    M_os << "(Result \""
         << M_left_team_name << "\" \""
         << M_right_team_name << "\" "
         << M_left_score << " "
         << M_right_score << ")"
         << std::endl;
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
TextPrinter::handleShow( const rcsc::rcg::ShowInfoT & show )
{
    if ( ! M_init_written )
    {
        M_init_written = true;
        M_os << "(Init)" << "\n";
    }

    M_os << "(Info ";
    printState( M_os, show.time_ );

    // ball
    M_os << " (ball "
         << show.ball_.x_ << ' ' << show.ball_.y_ << ' '
         << show.ball_.vx_ << ' ' << show.ball_.vy_ << ')';

    // players
    for ( int i = 0; i < rcsc::MAX_PLAYER*2; ++i )
    {
        rcsc::rcg::player_t p;
        rcsc::rcg::convert( show.player_[i], p );

        printPlayer( M_os,
                     show.player_[i].side(),
                     show.player_[i].unum_,
                     M_command_count[i],
                     p );
        M_command_count[i].update( show.player_[i] );
    }

    M_os << ")\n";
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
TextPrinter::handleMsg( const int,
                        const int,
                        const std::string & )
{
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
TextPrinter::handleDraw( const int,
                         const rcsc::rcg::drawinfo_t & )
{
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
TextPrinter::handlePlayMode( const int,
                             const rcsc::PlayMode pm )
{
    M_playmode = pm;
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
TextPrinter::handleTeam( const int,
                         const rcsc::rcg::TeamT & team_l,
                         const rcsc::rcg::TeamT & team_r )
{
    if ( M_left_team_name.empty() )
    {
        M_left_team_name = team_l.name_;
    }
    if ( M_right_team_name.empty() )
    {
        M_right_team_name = team_r.name_;
    }

    M_left_score = team_l.score_;
    M_right_score = team_r.score_;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
TextPrinter::handleServerParam( const std::string & msg )
{
    std::string::size_type pos = msg.find_first_of( ' ' );
    if ( pos != std::string::npos )
    {
        M_init_written = true;
        M_os << "(Init"
             << msg.substr( pos )
             << '\n';
    }
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
TextPrinter::handlePlayerParam( const std::string & )
{
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
TextPrinter::handlePlayerType( const std::string & )
{
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
const
std::string &
TextPrinter::getPlayModeString( const rcsc::PlayMode playmode ) const
{
    static const std::string s_playmode_str[] = {
        "unknown playmode",
        "before_kick_off",
        "time_over",
        "play_on",
        "kick_off_l",
        "kick_off_r",
        "kick_in_l",
        "kick_in_r",
        "free_kick_l",
        "free_kick_r",
        "corner_kick_l",
        "corner_kick_r",
        "goal_kick_l",
        "goal_kick_r",
        "goal_l",
        "goal_r",
        "drop_ball",
        "offside_l",
        "offside_r",
        "penalty_kick_l",
        "penalty_kick_r",
        "first_half_over",
        "pause",
        "human_judge",
        "foul_charge_l",
        "foul_charge_r",
        "foul_push_l",
        "foul_push_r",
        "foul_multiple_attack_l",
        "foul_multiple_attack_r",
        "foul_ballout_l",
        "foul_ballout_r",
        "back_pass_l",
        "back_pass_r",
        "free_kick_fault_l",
        "free_kick_fault_r",
        "catch_fault_l",
        "catch_fault_r",
        "indirect_free_kick_l",
        "indirect_free_kick_r",
        "penalty_setup_l",
        "penalty_setup_r",
        "penalty_ready_l",
        "penalty_ready_r",
        "penalty_taken_l",
        "penalty_taken_r",
        "penalty_miss_l",
        "penalty_miss_r",
        "penalty_score_l",
        "penalty_score_r",
        ""
    };


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
TextPrinter::printState( std::ostream & os,
                         const long & cycle ) const
{
    os << "(state "
       << cycle << " "
       << getPlayModeString( M_playmode ) << " "
       << M_left_score << " "
       << M_right_score << ")";
    return os;
}

/*-------------------------------------------------------------------*/
/*!
  old log format
*/
std::ostream &
TextPrinter::printBall( std::ostream & os,
                        const rcsc::rcg::pos_t & ball ) const
{
    os << "(ball "
       << rcsc::rcg::nstohd( ball.x ) << " "
       << rcsc::rcg::nstohd( ball.y ) << ")";

    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
TextPrinter::printBall( std::ostream & os,
                        const rcsc::rcg::ball_t & ball ) const
{
    os << "(ball "
       << rcsc::rcg::nltohd( ball.x ) << " "
       << rcsc::rcg::nltohd( ball.y ) << " "
       << rcsc::rcg::nltohd( ball.deltax ) << " "
       << rcsc::rcg::nltohd( ball.deltay ) << ")";

    return os;
}


/*-------------------------------------------------------------------*/
/*!
  old log format
*/
std::ostream &
TextPrinter::printPlayer( std::ostream & os,
                          //const std::string & teamname,
                          const rcsc::SideID side,
                          const int unum,
                          const rcsc::rcg::pos_t & player ) const
{
    const short mode = ntohs( player.enable );

    os << "(player "
        //<< teamname << " "
       << ( side == rcsc::LEFT ? "l " : "r " )
       << unum;
    if ( mode & rcsc::rcg::GOALIE )
    {
        os << " g";
    }
    os << " (position "
       << rcsc::rcg::nstohd( player.x ) << " "
       << rcsc::rcg::nstohd( player.y ) << " "
       << static_cast< double >( static_cast< short >( ntohs( player.angle ) ) )
       << ")";

    ///////////////////////////////////////////////////////

    if ( mode & rcsc::rcg::KICK_FAULT )
    {
        os << " (kick fault)";
    }
    if ( mode & rcsc::rcg::KICK )
    {
        os << " (kick)";
    }
    else if ( mode & rcsc::rcg::TACKLE_FAULT )
    {
        os << " (tackle fault)";
    }
    else if ( mode & rcsc::rcg::TACKLE )
    {
        os << " (tackle)";
    }
    else if ( mode & rcsc::rcg::CATCH_FAULT )
    {
        os << " (catch fault)";
    }
    else if ( mode & rcsc::rcg::CATCH )
    {
        os << " (catch)";
    }

    os << ")";
    return os;
}


/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
TextPrinter::printPlayer( std::ostream & os,
                          //const std::string & teamname,
                          const rcsc::SideID side,
                          const int unum,
                          const TextPrinter::CommandCount & count,
                          const rcsc::rcg::player_t & player ) const
{
    const short mode = ntohs( player.mode );

    double body_deg
        = rcsc::rcg::nltohd( player.body_angle )
        * (180.0 / M_PI);
    double head_deg
        = rcsc::rcg::nltohd( player.head_angle )
        * (180.0 / M_PI);
    head_deg += body_deg;
    while ( head_deg > 180.0 )
    {
        head_deg -= 360.0;
    }
    while ( head_deg < -180.0 )
    {
        head_deg += 360.0;
    }

    ///////////////////////////////////////////////////////
    os << "(player "
        //<< teamname << " "
       << ( side == rcsc::LEFT ? "l " : "r " )
       << unum;
    if ( mode & rcsc::rcg::GOALIE )
    {
        os << " g";
    }
    os << " (position "
       << rcsc::rcg::nltohd( player.x ) << " "
       << rcsc::rcg::nltohd( player.y ) << " "
       << rcsc::rcg::nltohd( player.deltax ) << " "
       << rcsc::rcg::nltohd( player.deltay ) << " "
       << body_deg << " "
       << head_deg << ")"; // global neck angle
    os << " (stamina "
       << rcsc::rcg::nltohd( player.stamina ) << ")";

    ///////////////////////////////////////////////////////

    if ( count.turn_ != rcsc::rcg::nstohi( player.turn_count ) )
    {
        os << " (turn)";
    }
    else if ( count.dash_ != rcsc::rcg::nstohi( player.dash_count ) )
    {
        os << " (dash)";
    }
    else if ( mode & rcsc::rcg::KICK_FAULT )
    {
        os << " (kick fault)";
    }
    else if ( mode & rcsc::rcg::KICK )
    {
        os << " (kick)";
    }
    else if ( mode & rcsc::rcg::TACKLE_FAULT )
    {
        os << " (tackle fault)";
    }
    else if ( mode & rcsc::rcg::TACKLE )
    {
        os << " (tackle)";
    }
    else if ( mode & rcsc::rcg::CATCH_FAULT )
    {
        os << " (catch fault)";
    }
    else if ( mode & rcsc::rcg::CATCH )
    {
        os << " (catch)";
    }
    else if ( count.move_ != rcsc::rcg::nstohi( player.move_count ) )
    {
        os << " (move)";
    }
    else
    {
        os << " (none)";
    }

    ///////////////////////////////////////////////////////

    if ( count.say_ != rcsc::rcg::nstohi( player.say_count ) )
    {
        os << " (say)";
    }
    if ( count.turn_neck_ != rcsc::rcg::nstohi( player.turn_neck_count ) )
    {
        os << " (turn_neck)";
    }
    if ( count.change_view_ != rcsc::rcg::nstohi( player.change_view_count ) )
    {
        os << " (change_view)";
    }

    ///////////////////////////////////////////////////////
    os << ")";
    return os;
}


////////////////////////////////////////////////////////////////////////

int
main( int argc, char** argv )
{
    if ( argc != 2
         || ! std::strncmp( argv[1], "--help", 6 )
         || ! std::strncmp( argv[1], "-h", 2 ) )
    {
        std::cerr << "usage: " << argv[0] << " <RcgFile>[.gz]" << std::endl;
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

    // create rcg handler instance
    TextPrinter printer( std::cout );

    parser->parse( fin, printer );

    return 0;
}
