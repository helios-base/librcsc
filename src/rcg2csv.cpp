// -*-c++-*-

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

#include <rcsc/common/server_param.h>
#include <rcsc/common/player_param.h>
#include <rcsc/common/player_type.h>

#include <rcsc/param/param_map.h>
#include <rcsc/param/cmd_line_parser.h>

#include <rcsc/types.h>
#include <rcsc/gz.h>
#include <rcsc/rcg.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <iostream>
#include <cmath>


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


    std::ostream & M_tracking_out;
    std::ostream & M_player_types_out;

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
    CSVPrinter( std::ostream & trakcing_out,
                std::ostream & player_types_out );

    bool handleLogVersion( const int ver ) override;

    bool handleEOF() override;

    bool handleShow( const rcsc::rcg::ShowInfoT & show ) override;
    bool handleMsg( const int time,
                    const int board,
                    const std::string & msg ) override;
    bool handleDraw( const int time,
                     const rcsc::rcg::drawinfo_t & draw ) override;
    bool handlePlayMode( const int time,
                         const rcsc::PlayMode pm ) override;
    bool handleTeam( const int time,
                     const rcsc::rcg::TeamT & team_l,
                     const rcsc::rcg::TeamT & team_r ) override;

    bool handleServerParam( const rcsc::rcg::ServerParamT & param ) override;
    bool handlePlayerParam( const rcsc::rcg::PlayerParamT & param ) override;
    bool handlePlayerType( const rcsc::rcg::PlayerTypeT & param ) override;

    bool handleTeamGraphic( const char,
                            const int,
                            const int,
                            const std::vector< std::string > & ) override
    {
        return true;
    }

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
CSVPrinter::CSVPrinter( std::ostream & tracking_out,
                        std::ostream & player_types_out)
    : M_tracking_out( tracking_out ),
      M_player_types_out( player_types_out ),
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
bool
CSVPrinter::handleServerParam( const rcsc::rcg::ServerParamT & param )
{
    rcsc::ServerParam::instance().convertFrom( param );
    return true;
}

/*-------------------------------------------------------------------*/
bool
CSVPrinter::handlePlayerParam( const rcsc::rcg::PlayerParamT & param )
{
    rcsc::PlayerParam::instance().convertFrom( param );
    return true;
}

/*-------------------------------------------------------------------*/
bool
CSVPrinter::handlePlayerType( const rcsc::rcg::PlayerTypeT & ptype )
{
    static bool s_header = false;
    if ( ! s_header )
    {
        M_player_types_out << "id,player_speed_max,stamina_inc_max,player_decay,inertia_moment,dash_power_rate,player_size,kickable_margin,kick_rand,extra_stamina,effort_max,effort_min,kick_power_rate,foul_detect_probability,catchable_area_l_stretch"
                       << '\n';
        s_header = true;
    }

    M_player_types_out << ptype.id_
                       << ',' << ptype.player_speed_max_
                       << ',' << ptype.stamina_inc_max_
                       << ',' << ptype.player_decay_
                       << ',' << ptype.inertia_moment_
                       << ',' << ptype.dash_power_rate_
                       << ',' << ptype.player_size_
                       << ',' << ptype.kickable_margin_
                       << ',' << ptype.kick_rand_
                       << ',' << ptype.extra_stamina_
                       << ',' << ptype.effort_max_
                       << ',' << ptype.effort_min_
                       << ',' << ptype.kick_power_rate_
                       << ',' << ptype.foul_detect_probability_
                       << ',' << ptype.catchable_area_l_stretch_
                       << '\n';
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
    return M_tracking_out;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CSVPrinter::printPlayerParam() const
{
    return M_tracking_out;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CSVPrinter::printPlayerTypes() const
{
    return M_player_types_out;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CSVPrinter::printShowHeader() const
{
    M_tracking_out << "#"
                   << ",cycle,stopped"
                   << ",playmode"
                   << ",l_name,l_score,l_pen_score"
                   << ",r_name,r_score,r_pen_score"
                   << ",b_x,b_y,b_vx,b_vy";

    char side = 'l';
    for ( int s = 0; s < 2; ++s )
    {
        for ( int i = 1; i <= rcsc::MAX_PLAYER; ++i )
        {
            M_tracking_out << "," << side << i << "_t"
                           << "," << side << i << "_x"
                           << "," << side << i << "_y"
                           << "," << side << i << "_vx"
                           << "," << side << i << "_vy"
                           << "," << side << i << "_body"
                           << "," << side << i << "_neck"
                           << "," << side << i << "_vwidth"
                           << "," << side << i << "_stamina"
                ;
        }
        side = 'r';
    }

    M_tracking_out << '\n';
    return M_tracking_out;
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

    M_tracking_out << '\n';
    return M_tracking_out;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CSVPrinter::printShowCount() const
{
    M_tracking_out << M_show_count;
    return M_tracking_out;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CSVPrinter::printTime() const
{
    M_tracking_out << ',' << M_cycle << ',' << M_stopped;
    return M_tracking_out;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CSVPrinter::printPlayMode() const
{
    M_tracking_out << ',' << std::quoted( getPlayModeString( M_playmode ) );
    return M_tracking_out;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CSVPrinter::printTeams() const
{
    for ( const auto & t : M_teams )
    {
        M_tracking_out << ',' << std::quoted( t.name_ )
                       << ',' << t.score_
                       << ',' << t.pen_score_;
    }

    return M_tracking_out;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CSVPrinter::printBall( const rcsc::rcg::BallT & ball ) const
{
    M_tracking_out << ',' << ball.x_
                   << ',' << ball.y_
                   << ',' << ball.vx_
                   << ',' << ball.vy_;
    return M_tracking_out;
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

    return M_tracking_out;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CSVPrinter::printPlayer( const rcsc::rcg::PlayerT & player ) const
{
    if ( player.state_ == rcsc::rcg::DISABLE )
    {
        M_tracking_out << ',' //<< player.type_
                       << ',' //<< player.x_
                       << ',' //<< player.y_
                       << ',' //<< player.vx_
                       << ',' //<< player.vy_
                       << ',' //<< player.body_
                       << ',' //<< player.neck_;
                       << ',' //<< player.view_width_;
                       << ',' //<< player.stamina_;
            ;
    }
    else
    {
        M_tracking_out << ',' << player.type_
                       << ',' << player.x_
                       << ',' << player.y_
                       << ',' << player.vx_
                       << ',' << player.vy_
                       << ',' << player.body_
                       << ',' << player.neck_
                       << ',' << player.view_width_
                       << ',' << player.stamina_
            ;
    }
    return M_tracking_out;
}

////////////////////////////////////////////////////////////////////////
std::string
get_base_name( const std::string & path )
{
    // remove all extension (".rcg" or ".rcg.gz")from file name
    std::filesystem::path p( path );

    // if file name has .gz extension, remove it
    if ( p.extension() == ".gz" )
    {
        p.replace_extension();
    }

    // if file name has .rcg extension, remove it
    if ( p.extension() == ".rcg" )
    {
        p.replace_extension();
    }

    return p.string();
}

////////////////////////////////////////////////////////////////////////

int
main( int argc, char** argv )
{
    bool help = false;
    bool print_player_types = false;

    rcsc::ParamMap options( "Options" );
    options.add()
        ( "help", "", rcsc::BoolSwitch( &help ), "print help message." )
        ( "player-types", "p", rcsc::BoolSwitch( &print_player_types ), "print player_type information."  )
        ;

    rcsc::CmdLineParser cmd_parser( argc, argv );
    cmd_parser.parse( options );

    if ( help
         || cmd_parser.failed()
         || cmd_parser.positionalOptions().empty() )
    {
        std::cerr << " usage:\n";
        std::cerr << "  " << argv[0] << " [-p] <RCGFile>[.gz] ...\n";
        options.printHelp( std::cerr );
        return 0;
    }

    const std::string infile = cmd_parser.positionalOptions().front();
    rcsc::gzifstream fin( infile.c_str() );

    if ( ! fin.is_open() )
    {
        std::cerr << "Failed to open file : " << infile << std::endl;
        return 1;
    }

    const std::string basename = get_base_name( infile );
    const std::string tracking_csv = basename + ".tracking.csv";
    const std::string player_types_csv = basename + ".player_types.csv";

    std::ofstream tracking_out( tracking_csv );
    if ( ! tracking_out.is_open() )
    {
        std::cerr << "Failed to open the output file : " << tracking_csv << std::endl;
        return 1;
    }

    std::ofstream player_types_out( player_types_csv );
    if ( ! player_types_out.is_open() )
    {
        std::cerr << "Failed to open the output file : " << player_types_csv << std::endl;
        return 1;
    }

    std::cerr << " in:           " << infile << '\n';
    std::cerr << " tracking:     " << tracking_csv << '\n';
    std::cerr << " player_types: " << player_types_csv << std::endl;

    rcsc::rcg::Parser::Ptr parser = rcsc::rcg::Parser::create( fin );

    if ( ! parser )
    {
        std::cerr << "Failed to create rcg parser." << std::endl;
        return 1;
    }

    CSVPrinter printer( tracking_out, player_types_out );

    parser->parse( fin, printer );

    return 0;
}
