// -*-c++-*-

/*!
  \file resultprinter.cpp
  \brief game result printer program source File.
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

#include <rcsc/gz/gzfstream.h>
#include <rcsc/rcg.h>

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <cstring>
#include <ctime>

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

struct Point {
    double x;
    double y;

    Point()
        : x( 0.0 ),
          y( 0.0 )
      { }
};

class ResultPrinter
    : public rcsc::rcg::Handler {
private:

    static const double PITCH_LENGTH;
    static const double PITCH_WIDTH;

    static const double GOAL_POST_RADIUS;

    std::string M_file_path;
    std::time_t M_game_date;

    double M_goal_width;
    double M_ball_size;
    int M_half_time;

    rcsc::PlayMode M_playmode;
    int M_cycle;

    std::string M_left_team_name; //!< left teamname string
    std::string M_right_team_name; //!< right teamname string

    int M_left_score; //!< left team score
    int M_right_score; //!< right team score

    int M_left_penalty_taken; //!< total number of left team penalty trial
    int M_right_penalty_taken; //!< total number of left team penalty trial

    int M_left_penalty_score; //!< left team penalty kick score
    int M_right_penalty_score; //!< left team penalty kick score

    rcsc::SideID M_last_penalty_taker_side;

    // not used
    ResultPrinter() = delete;
    ResultPrinter( const ResultPrinter & ) = delete;
    ResultPrinter & operator=( const ResultPrinter & ) = delete;

public:

    explicit
    ResultPrinter( const std::string & input_file );

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

    bool crossGoalLine( const Point & ball_pos,
                        const Point & prev_ball_pos );

    void checkFinalPenaltyGoal( const Point & ball_pos );

};

const double ResultPrinter::PITCH_LENGTH = 105.0;
const double ResultPrinter::PITCH_WIDTH = 68.0;

const double ResultPrinter::GOAL_POST_RADIUS = 0.06;

/*-------------------------------------------------------------------*/
/*!

*/
ResultPrinter::ResultPrinter( const std::string & input_file )
    : M_game_date( 0 ),
      M_goal_width( 14.02 ),
      M_ball_size( 0.085 ),
      M_half_time( 3000 ),
      M_playmode( rcsc::PM_Null ),
      M_cycle( 0 ),
      M_left_team_name( "" ),
      M_right_team_name( "" ),
      M_left_score( 0 ),
      M_right_score( 0 ),
      M_left_penalty_taken( 0 ),
      M_right_penalty_taken( 0 ),
      M_left_penalty_score( 0 ),
      M_right_penalty_score( 0 ),
      M_last_penalty_taker_side( rcsc::NEUTRAL )
{
    std::string::size_type pos = input_file.find_last_of( '/' );
    std::string base_name = ( pos == std::string::npos
                              ? input_file
                              : input_file.substr( pos + 1 ) );

    tm t;
    if ( strptime( base_name.c_str(), "%Y%m%d%H%M", &t ) )
    {
        t.tm_sec = 0;
        M_game_date = std::mktime( &t );
        //std::cerr << "file=" << argv[i] << std::endl;
        //std::cerr << "date=" << std::asctime( &t ) << std::endl;;
        //std::cerr << "date=" << std::ctime( &M_game_date ) << std::endl;;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ResultPrinter::crossGoalLine( const Point & ball_pos,
                              const Point & prev_ball_pos )
{
    double delta_x = ball_pos.x - prev_ball_pos.x;
    double delta_y = ball_pos.y - prev_ball_pos.y;

    double gradient = delta_y / delta_x;
    double offset = prev_ball_pos.y - gradient * prev_ball_pos.x;

    double x = PITCH_LENGTH*0.5 + M_ball_size;
    if ( ball_pos.x < 0.0 ) x *= -1.0;
    double y_intercept = gradient * x + offset;

    //     std::cout << ": prev = "
    //               << prev_ball_pos.x << ','
    //               << prev_ball_pos.y
    //               << std::endl;
    //     std::cout << ": curr = "
    //               << ball_pos.x << ','
    //               << ball_pos.y
    //               << std::endl;
    //     std::cout << ": delta_x = " << delta_x << std::endl;
    //     std::cout << ": delta_y = " << delta_y << std::endl;
    //     std::cout << ": grad = " << gradient << std::endl;
    //     std::cout << ": off = " << offset << std::endl;
    //     std::cout << ": x = " << x << std::endl;
    //     std::cout << ": y_inter = " << y_intercept << std::endl;

    return ( std::fabs( y_intercept ) <= ( M_goal_width*0.5 + GOAL_POST_RADIUS ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ResultPrinter::checkFinalPenaltyGoal( const Point & ball_pos )
{
    static Point s_prev_ball_pos;

    if ( M_playmode == rcsc::PM_TimeOver
         && crossGoalLine( ball_pos, s_prev_ball_pos ) )
    {
        if ( M_last_penalty_taker_side == rcsc::LEFT )
        {
            //std::cerr << "time_over -> penalty_score_l" << std::endl;
            ++M_left_penalty_score;
        }
        else if ( M_last_penalty_taker_side == rcsc::RIGHT )
        {
            //std::cerr << "time_over -> penalty_score_r" << std::endl;
            ++M_right_penalty_score;
        }
    }

    s_prev_ball_pos = ball_pos;
}

/*-------------------------------------------------------------------*/
/*!
  print result
  "<TeamNameL> <TeamNameR> <ScoreL> <ScoreR>
*/
bool
ResultPrinter::handleEOF()
{
    bool incomplete = false;

    if ( M_left_team_name.empty() )
    {
        M_left_team_name = "null";
        incomplete = true;
    }

    if ( M_right_team_name.empty() )
    {
        M_right_team_name = "null";
        incomplete = true;
    }

    char date[256];
    std::strftime( date, 255, "%Y%m%d%H%M%S", localtime( &M_game_date ) );
    std::cout << date << ' ';

    std::cout << M_left_team_name << " " << M_right_team_name << " "
              << M_left_score << " " << M_right_score;

    if ( M_left_penalty_taken > 0
         && M_right_penalty_taken > 0 )
    {
        std::cout << " " << M_left_penalty_score
                  << " " << M_right_penalty_score;
    }

    if ( ! incomplete
         && M_playmode != rcsc::PM_TimeOver )
    {
        if ( ( M_cycle % M_half_time == 0 // just a half time
               || ( M_cycle + 1 ) % M_half_time == 0 )
             && ( ( M_cycle / M_half_time ) % 2 == 0 // even number halves
                  || ( ( M_cycle + 1 ) / M_half_time ) % 2 == 0 )
             && M_left_score == M_right_score ) // draw game
        {

        }
        else
        {
            incomplete = true;
        }
    }

    if ( incomplete )
    {
        std::cout << " (incomplete match : cycle="
                  << M_cycle << ")";
    }

    std::cout << std::endl;

    return true;
}


/*-------------------------------------------------------------------*/
/*!

*/
bool
ResultPrinter::handleShow( const rcsc::rcg::ShowInfoT & show )
{
    M_cycle = static_cast< int >( show.time_ );

    if ( M_last_penalty_taker_side != rcsc::NEUTRAL )
    {
        Point ball_pos;

        ball_pos.x = show.ball_.x_;
        ball_pos.y = show.ball_.y_;

        checkFinalPenaltyGoal( ball_pos );
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ResultPrinter::handleMsg( const int,
                          const int,
                          const std::string & msg )
{
    if ( ! msg.compare( 0, 8, "(result " ) )
    {
        tm t;
        if ( strptime( msg.c_str(), "(result %Y%m%d%H%M%S ", &t ) )
        {
            M_game_date = std::mktime( &t );
        }
        else if ( strptime( msg.c_str(), "(result %Y%m%d%H%M ", &t ) )
        {
            t.tm_sec = 0;
            M_game_date = std::mktime( &t );
            //std::cerr << "date=" << std::asctime( &t ) << std::endl;;
            //std::cerr << "date=" << std::ctime( &M_game_date ) << std::endl;;
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ResultPrinter::handlePlayMode( const int,
                               const rcsc::PlayMode pm )
{
    if ( M_playmode == pm )
    {
        return true;
    }

    M_playmode = pm;

    switch ( M_playmode ) {
    case rcsc::PM_PenaltySetup_Left:
        ++M_left_penalty_taken;
        M_last_penalty_taker_side = rcsc::LEFT;
        break;
    case rcsc::PM_PenaltySetup_Right:
        ++M_right_penalty_taken;
        M_last_penalty_taker_side = rcsc::RIGHT;
        break;
    case rcsc::PM_PenaltyMiss_Left:
        break;
    case rcsc::PM_PenaltyMiss_Right:
        break;
    case rcsc::PM_PenaltyScore_Left:
        ++M_left_penalty_score;
        break;
    case rcsc::PM_PenaltyScore_Right:
        ++M_right_penalty_score;
        break;
    case rcsc::PM_TimeOver:
        break;
    default:
        break;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ResultPrinter::handleTeam( const int,
                           const rcsc::rcg::TeamT & team_l,
                           const rcsc::rcg::TeamT & team_r )
{
    M_left_team_name = team_l.name_;
    M_left_score = team_l.score_;
    M_left_penalty_taken = team_l.pen_score_ + team_l.pen_miss_;
    M_left_penalty_score = team_l.pen_score_;

    M_right_team_name = team_r.name_;
    M_right_score = team_r.score_;
    M_right_penalty_taken = team_r.pen_score_ + team_r.pen_miss_;
    M_right_penalty_score = team_r.pen_score_;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ResultPrinter::handleServerParam( const std::string & line )
{
    int n_read = 0;

    char message_name[32];
    if ( std::sscanf( line.c_str(), " ( %31s %n ", message_name, &n_read ) != 1 )
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << ":error: failed to the parse message id." << std::endl;
        return false;
    }

    for ( std::string::size_type pos = line.find_first_of( '(', n_read );
          pos != std::string::npos;
          pos = line.find_first_of( '(', pos ) )
    {
        std::string::size_type end_pos = line.find_first_of( ' ', pos );
        if ( end_pos == std::string::npos )
        {
            std::cerr << __FILE__ << ' ' << __LINE__
                      << ":error: failed to find parameter name." << std::endl;
            return false;
        }
        pos += 1;

        const std::string name_str( line, pos, end_pos - pos );
        pos = end_pos;

        // search end paren or double quatation
        end_pos = line.find_first_of( ")\"", end_pos ); //"
        if ( end_pos == std::string::npos )
        {
            std::cerr << __FILE__ << ' ' << __LINE__
                      << ":error: failed to parse parameter value for ["
                      << name_str << "] " << std::endl;
            return false;
        }

        // found quated value
        if ( line[end_pos] == '\"' )
        {
            pos = end_pos;
            end_pos = line.find_first_of( '\"', end_pos + 1 ); //"
            if ( end_pos == std::string::npos )
            {
                std::cerr << __FILE__ << ' ' << __LINE__
                          << ":error: ailed to parse the quated value for ["
                          << name_str << "] " << std::endl;
                return false;
            }
            end_pos += 1; // skip double quatation
        }
        else
        {
            pos += 1; // skip white space
        }

        const std::string value_str( line, pos, end_pos - pos );
        pos = end_pos;

        try
        {
            if ( name_str == "goal_width" )
            {
                M_goal_width = std::stod( value_str );
            }
            else if ( name_str == "ball_size" )
            {
                M_ball_size = std::stod( value_str );
            }
            else if ( name_str == "half_time" )
            {
                M_half_time = std::stoi( value_str );
            }
        }
        catch ( std::exception & e )
        {
            std::cerr << __FILE__ << ' ' << __LINE__
                      << ": Exeption caught! " << e.what() << std::endl;
            return false;
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ResultPrinter::handlePlayerParam( const std::string & )
{
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ResultPrinter::handlePlayerType( const std::string & )
{
    return true;
}

////////////////////////////////////////////////////////////////////////

void
usage( const char * prog )
{
    std::cerr << "Usage: " << prog << " <RcgFile>[.gz]"
              << std::endl;
}


////////////////////////////////////////////////////////////////////////

int
main( int argc, char** argv )
{
    if ( argc < 2 )
    {
        usage( argv[0] );
        return 1;
    }

    for ( int i = 1; i < argc; ++i )
    {
        if ( argv[i][0] == '-' )
        {
            continue;
        }

        const std::string file = argv[i];

        rcsc::gzifstream fin( file.c_str() );

        if ( ! fin.is_open() )
        {
            std::cerr << "Failed to open file : " << file
                      << std::endl;
            continue;
        }

        rcsc::rcg::Parser::Ptr parser = rcsc::rcg::Parser::create( fin );

        if ( ! parser )
        {
            std::cerr << "Failed to create rcg parser for "
                      << argv[i]
                      << std::endl;
            continue;
        }

        // create rcg handler instance
        ResultPrinter printer( file );

        if ( ! parser->parse( fin, printer ) )
        {
            std::cerr << "Failed to parse [" << argv[i] << "]"
                      << std::endl;
        }

        fin.close();
    }

    return 0;
}
