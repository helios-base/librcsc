// -*-c++-*-

/*!
  \file tableprinter.cpp
  \brief game result table printer program source File.
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

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <list>
#include <map>
#include <cstdio>
#include <cstring>

////////////////////////////////////////////////////////////////////////

struct Match {
    typedef std::pair< std::string, std::string > Key;

    std::string date_;
    std::string name_l_;
    std::string name_r_;
    int score_l_;
    int score_r_;
    int pen_score_l_;
    int pen_score_r_;
    bool reversed_;

    Match( const std::string & date,
           const std::string & name_l,
           const int score_l,
           const int pen_score_l,
           const std::string & name_r,
           const int score_r,
           const int pen_score_r,
           const bool reversed = false )
        : date_( date ),
          name_l_( name_l ),
          name_r_( name_r ),
          score_l_( score_l ),
          score_r_( score_r ),
          pen_score_l_( pen_score_l ),
          pen_score_r_( pen_score_r ),
          reversed_( reversed )
      { }

    const std::string & realLeftTeamName() const
      {
          return reversed_ ? name_r_ : name_l_;
      }
    const std::string & realRightTeamName() const
      {
          return reversed_ ? name_l_ : name_r_;
      }
    int realLeftScore() const
      {
          return reversed_ ? score_r_ : score_l_;
      }
    int realRightScore() const
      {
          return reversed_ ? score_l_ : score_r_;
      }
    int realLeftPenScore() const
      {
          return reversed_ ? pen_score_r_ : pen_score_l_;
      }
    int realRightPenScore() const
      {
          return reversed_ ? pen_score_l_ : pen_score_r_;
      }

    bool hasPenaltyScore() const
      {
          return pen_score_l_ + pen_score_r_ > 0;
      }
};

typedef std::map< Match::Key, Match > MatchTable;

////////////////////////////////////////////////////////////////////////

struct Team {
    std::string name_;
    int games_;
    int win_;
    int lose_;
    int draw_;
    int points_;
    int goal_scored_;
    int goal_conceded_;
    int tied_goal_scored_;
    int tied_goal_conceded_;
    bool tie_;

    Team( const std::string & name )
        : name_( name ),
          games_( 0 ),
          win_( 0 ),
          lose_( 0 ),
          draw_( 0 ),
          points_( 0 ),
          goal_scored_( 0 ),
          goal_conceded_( 0 ),
          tied_goal_scored_( 0 ),
          tied_goal_conceded_( 0 ),
          tie_( false )
      { }

    int goalDiff() const
      {
          return goal_scored_ - goal_conceded_;
      }

    int tiedGoalDiff() const
      {
          return tied_goal_scored_ - tied_goal_conceded_;
      }
};


////////////////////////////////////////////////////////////////////////

struct TeamNameCmp {

    bool operator()( const Team & lhs,
                     const Team & rhs ) const
      {
          return lhs.name_ < rhs.name_;
      }
};


////////////////////////////////////////////////////////////////////////

struct PointCmp {

    bool operator()( const Team & lhs,
                     const Team & rhs ) const
      {
          return static_cast< double >( lhs.points_ ) / static_cast< double >( lhs.games_ )
              > static_cast< double >( rhs.points_ ) / static_cast< double >( rhs.games_ );
      }
};

////////////////////////////////////////////////////////////////////////

struct GoalDiffCmp {

    bool operator()( const Team & lhs,
                     const Team & rhs ) const
      {
          return static_cast< double >( lhs.goalDiff() ) / static_cast< double >( lhs.games_ )
              > static_cast< double >( rhs.goalDiff() ) / static_cast< double >( rhs.games_ );
      }
};

////////////////////////////////////////////////////////////////////////

struct TiedGoalDiffCmp {

    bool operator()( const Team & lhs,
                     const Team & rhs ) const
      {
          return lhs.tiedGoalDiff()
              > rhs.tiedGoalDiff();
      }
};

////////////////////////////////////////////////////////////////////////

struct GoalScoredCmp {

    bool operator()( const Team & lhs,
                     const Team & rhs ) const
      {
          return lhs.goal_scored_ > rhs.goal_scored_;
      }
};

////////////////////////////////////////////////////////////////////////

struct TiedGoalScoredCmp {

    bool operator()( const Team & lhs,
                     const Team & rhs ) const
      {
          return lhs.tied_goal_scored_ > rhs.tied_goal_scored_;
      }
};


////////////////////////////////////////////////////////////////////////

class TablePrinter {
public:
      enum PrintType {
          PukiWiki,
          HTML,
          XML,
          NO_STYLE,
      };
private:
    std::string M_input_file;
    std::string M_output_file;
    std::string M_group_name;
    std::string M_log_dir;
    PrintType M_print_type;

    std::list< Team > M_teams;
    std::list< Match > M_match_list;
    MatchTable M_match_table;

public:

    TablePrinter();
    ~TablePrinter();

    bool parseCmdLine( int argc,
                       char ** argv );

    bool read();
    bool print() const;
private:

    Team * getTeam( const std::string & name );
    int calcPoint( const int our_score,
                   const int our_pen_score,
                   const int opp_score,
                   const int opp_pen_score ) const;
    bool addMatch( const std::string & date,
                   const std::string & name_l,
                   const int score_l,
                   const int pen_score_l,
                   const std::string & name_r,
                   const int score_r,
                   const int pen_score_r );

    void sortTeams();
    void sortCopyTwoTeams( std::list< Team > & sorted_teams,
                           std::list< Team > & teams );
    void sortCopyTieTeams( std::list< Team > & sorted_teams,
                           std::list< Team > & teams );
    void updateTiedGroupGoals( std::list< Team > & teams );

    std::ostream & printPukiWiki( std::ostream & os ) const;
    std::ostream & printHtml( std::ostream & os ) const;
    std::ostream & printXml( std::ostream & os ) const;
};

/*-------------------------------------------------------------------*/
/*!

 */
TablePrinter::TablePrinter()
    : M_group_name( "Group" ),
      M_log_dir( "" ),
      M_print_type( PukiWiki )
{

}


/*-------------------------------------------------------------------*/
/*!

 */
TablePrinter::~TablePrinter()
{

}

/*-------------------------------------------------------------------*/
/*!

 */
bool
TablePrinter::parseCmdLine( int argc,
                            char ** argv )
{
    bool help = false;
    bool usage = false;

    if ( argc <= 1 )
    {
        help = true;
        usage = true;
    }

    for ( int i = 1; i < argc; ++i )
    {
        if ( ! std::strncmp( argv[i], "--input", 7 ) )
        {
            if ( i + 1 >= argc )
            {
                usage = true;
                break;
            }

            M_input_file = argv[i+1];
            ++i;
        }
        else if ( ! std::strncmp( argv[i], "--output", 8 ) )
        {
            if ( i + 1 >= argc )
            {
                usage = true;
                break;
            }

            M_output_file = argv[i+1];
            ++i;
        }
        else if ( ! std::strncmp( argv[i], "--group", 7 ) )
        {
            if ( i + 1 >= argc )
            {
                usage = true;
                break;
            }

            M_group_name = argv[i+1];
            ++i;
        }
        else if ( ! std::strncmp( argv[i], "--log-dir", 9 ) )
        {
            if ( i + 1 >= argc )
            {
                usage = true;
                break;
            }

            M_log_dir = argv[i+1];
            ++i;
        }
        else if ( ! std::strncmp( argv[i], "--type", 7 ) )
        {
            if ( i + 1 >= argc )
            {
                usage = true;
                break;
            }

            if ( ! std::strncmp( argv[i+1], "pukiwiki", 8 ) )
            {
                M_print_type = PukiWiki;
            }
            else if ( ! std::strncmp( argv[i+1], "html", 4 ) )
            {
                M_print_type = HTML;
            }
            else if ( ! std::strncmp( argv[i+1], "xml", 3 ) )
            {
                M_print_type = XML;
            }
            else
            {
                std::cerr << "unknown print type " << argv[i+1] << std::endl;
                usage = true;
                break;
            }
            ++i;
        }
        else if ( ! std::strncmp( argv[i], "--help", 6 ) )
        {
            help = true;
            break;
        }
        else if ( ! std::strncmp( argv[i], "--", 2 ) )
        {
            usage = true;
            break;
        }
        else
        {
            M_input_file = argv[i];
        }
    }

    if ( usage || help )
    {
        std::cerr << "Usage: " << argv[0] << " [options ... ] [<ResultsFile>]\n"
                  << '\n'
                  << "Allowed options:\n"
                  << "  --help            print this message.\n"
                  << "  --input <value>   set an input file path. if empty or '-', stdin is used.\n"
                  << "  --output <value>  set an output file path. if empty or '-', stdout is used.\n"
                  << "  --group <value>   set a group name.\n"
                  << "  --log-dir <value> set a log file location.\n"
                  << "  --type <value>    set a print type {pukiwiki,html}.\n"
                  << std::flush;
        return false;
    }

    if ( M_log_dir.empty() )
    {
        M_log_dir = "./";
    }
    else if ( *M_log_dir.rbegin() != '/' )
    {
        M_log_dir += '/';
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
TablePrinter::read()
{
    std::istream * is = &std::cin;

    if ( ! M_input_file.empty()
         && M_input_file != "-" )
    {
        is = new std::ifstream( M_input_file.c_str() );
        if ( ! *is )
        {
            std::cerr << "Error: could not open the input file. " << M_input_file
                      << std::endl;
            delete is;
            return false;
        }
    }

    int n_line = 0;
    std::string line;
    while ( std::getline( *is, line ) )
    {
        ++n_line;

        char date[256];
        char name_l[256];
        char name_r[256];
        int score_l = 0;
        int score_r = 0;
        int pen_score_l = 0;
        int pen_score_r = 0;
        int n_read = 0;

        int n = std::sscanf( line.c_str(),
                             " %255s %255s %255s %d %d %d %d %n ",
                             date,
                             name_l, name_r,
                             &score_l, &score_r,
                             &pen_score_l, &pen_score_r,
                             &n_read );

        if ( n != 5 && n != 7 )
        {
            std::cerr << n_line << ": illegal result format : " << line << std::endl;
            continue;
        }

        if ( line.find( "incomplete match" ) != std::string::npos )
        {
            std::cerr << n_line << ": incomplete match : " << line << std::endl;
            continue;
        }

        addMatch( date,
                  name_l, score_l, pen_score_l,
                  name_r, score_r, pen_score_r );
    }

    if ( is != &std::cin )
    {
        delete is;
        is = &std::cin;
    }

    sortTeams();

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
Team *
TablePrinter::getTeam( const std::string & name )
{
    for ( Team & t : M_teams )
    {
        if ( t.name_ == name )
        {
            return &t;
        }
    }

    M_teams.emplace_back( name );

    return &M_teams.back();
}

/*-------------------------------------------------------------------*/
/*!

 */
int
TablePrinter::calcPoint( const int our_score,
                         const int our_pen_score,
                         const int opp_score,
                         const int opp_pen_score ) const
{
    return ( our_score > opp_score
             ? 3
             : our_score < opp_score
             ? 0
             : our_pen_score > opp_pen_score
             ? 3
             : our_pen_score < opp_pen_score
             ? 0
             : 1 );
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
TablePrinter::addMatch( const std::string & date,
                        const std::string & name_l,
                        const int score_l,
                        const int pen_score_l,
                        const std::string & name_r,
                        const int score_r,
                        const int pen_score_r )
{
    //
    // check existing matches
    //

    if ( M_match_table.find( Match::Key( name_l, name_r ) ) != M_match_table.end()
         || M_match_table.find( Match::Key( name_r, name_l ) ) != M_match_table.end() )
    {
        std::cerr << "[" << name_l << "-vs-" << name_r << "] already exists." << std::endl;
        return false;
    }

    //
    // get or create new team data
    //

    Team * team_l = getTeam( name_l );
    if ( ! team_l )
    {
        return false;
    }

    Team * team_r = getTeam( name_r );
    if ( ! team_r )
    {
        return false;
    }

    //
    // update team data
    //
    int point = calcPoint( score_l, pen_score_l, score_r, pen_score_r );
    team_l->games_ += 1;
    team_l->points_ += point;
    team_l->win_ += ( point == 3 ? 1 : 0 );
    team_l->lose_ += ( point == 0 ? 1 : 0 );
    team_l->draw_ += ( point == 1 ? 1 : 0 );
    team_l->goal_scored_ += score_l;
    team_l->goal_conceded_ += score_r;

    point = calcPoint( score_r, pen_score_r, score_l, pen_score_l );
    team_r->games_ += 1;
    team_r->points_ += point;
    team_r->win_ += ( point == 3 ? 1 : 0 );
    team_r->lose_ += ( point == 0 ? 1 : 0 );
    team_r->draw_ += ( point == 1 ? 1 : 0 );
    team_r->goal_scored_ += score_r;
    team_r->goal_conceded_ += score_l;


    //
    // addt to the match list
    //

    M_match_list.emplace_back( date,
                               name_l, score_l, pen_score_l,
                               name_r, score_r, pen_score_r );

    //
    // insert to the match table
    //

    M_match_table.insert( std::pair< Match::Key, Match >( Match::Key( name_l, name_r ),
                                                          Match( date,
                                                                 name_l, score_l, pen_score_l,
                                                                 name_r, score_r, pen_score_r ) ) );
    M_match_table.insert( std::pair< Match::Key, Match >( Match::Key( name_r, name_l ),
                                                          Match( date,
                                                                 name_r, score_r, pen_score_r,
                                                                 name_l, score_l, pen_score_l,
                                                                 true ) ) );

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
TablePrinter::sortTeams()
{
    /*
      1. points
      2. head-to-head result
         for n = 2, this breaks the tie if and only if the head to head match was not a draw
         for n > 2, this breaks the tie if and only if one team won against all other teams who are tied.
      3. Overall goal difference for this round
      4. if n > 2, overall goal difference including only games with the tied teams
      5. Overall number of goals scored
      6. if n > 2, overall number of goals scored including only games with the tied teams
      7. Penalty shootouts between the tied teams. Details of the structure will be explained further below.
     */

    M_teams.sort( PointCmp() );

    std::list< Team > sorted_teams;
    while ( M_teams.size() > 1 )
    {
        std::list< Team > tmp_teams;
        tmp_teams.splice( tmp_teams.end(), M_teams, M_teams.begin() );

        while ( ! M_teams.empty() )
        {
            if ( M_teams.front().points_ != tmp_teams.back().points_ )
            {
                break;
            }

            tmp_teams.splice( tmp_teams.end(), M_teams, M_teams.begin() );
        }

        sortCopyTieTeams( sorted_teams, tmp_teams );
    }

    sorted_teams.splice( sorted_teams.end(), M_teams );
    M_teams.swap( sorted_teams );

}

/*-------------------------------------------------------------------*/
/*!
  recursive function
 */
void
TablePrinter::sortCopyTieTeams( std::list< Team > & sorted_teams,
                                std::list< Team > & teams )
{
    if ( teams.size() <= 1 )
    {
        sorted_teams.splice( sorted_teams.end(), teams );
        return;
    }

    //std::cerr << "sortCopyTieTeams  size=" << teams.size() << std::endl;

    //
    // Rule.2: check head-to-head results
    //
    {
        std::list< Team >::iterator t1 = teams.begin();
        while ( t1 != teams.end() )
        {
            bool all_win = false;

            for ( std::list< Team >::iterator t2 = teams.begin();
                  t2 != teams.end();
                  ++t2 )
            {
                if ( &(*t1) == &(*t2) ) continue;

                MatchTable::const_iterator m = M_match_table.find( Match::Key( t1->name_, t2->name_ ) );
                if ( m == M_match_table.end() ) continue;

                if ( m->second.score_l_ + m->second.pen_score_l_
                     <= m->second.score_r_ + m->second.pen_score_r_ )
                {
                    all_win = false;
                    break;
                }

                all_win = true;
            }

            if ( all_win )
            {
                //std::cerr << "sortCopyTieTeams  exists all win team=" << t1->name_ << std::endl;
                sorted_teams.splice( sorted_teams.end(), teams, t1 );
                sortCopyTieTeams( sorted_teams, teams ); // recursive operation
                return;
            }
            else
            {
                ++t1;
            }
        }
    }

    //
    // Rule.3: check overall goal difference for this round
    //
    {
        teams.sort( GoalDiffCmp() );

        if ( teams.front().goalDiff() != teams.back().goalDiff() )
        {
            std::list< Team > tmp_teams;
            tmp_teams.splice( tmp_teams.end(), teams, teams.begin() );

            while ( ! teams.empty() )
            {
                if ( teams.front().goalDiff() != tmp_teams.back().goalDiff() )
                {
                    break;
                }

                tmp_teams.splice( tmp_teams.end(), teams, teams.begin() );
            }

            sortCopyTieTeams( sorted_teams, tmp_teams ); // recursive operation
            sortCopyTieTeams( sorted_teams, teams ); // recursive operation
            return;
        }
    }

    //
    // Rule.4: check overall goal difference including only games with tied teams
    //
    if ( teams.size() > 2 )
    {
        updateTiedGroupGoals( teams );
        teams.sort( TiedGoalDiffCmp() );

        if ( teams.front().tiedGoalDiff() != teams.back().tiedGoalDiff() )
        {
            std::list< Team > tmp_teams;
            tmp_teams.splice( tmp_teams.end(), teams, teams.begin() );

            while ( ! teams.empty() )
            {
                if ( teams.front().tiedGoalDiff() != tmp_teams.back().tiedGoalDiff() )
                {
                    break;
                }

                tmp_teams.splice( tmp_teams.end(), teams, teams.begin() );
            }

            sortCopyTieTeams( sorted_teams, tmp_teams ); // recursive operation
            sortCopyTieTeams( sorted_teams, teams ); // recursive operation
            return;
        }
    }

    //
    // Rule.5: check number of goal scored for this round
    //
    {
        teams.sort( GoalScoredCmp() );

        if ( teams.front().goal_scored_ != teams.back().goal_scored_ )
        {
            std::list< Team > tmp_teams;
            tmp_teams.splice( tmp_teams.end(), teams, teams.begin() );

            while ( ! teams.empty() )
            {
                if ( teams.front().goal_scored_ != tmp_teams.back().goal_scored_ )
                {
                    break;
                }

                tmp_teams.splice( tmp_teams.end(), teams, teams.begin() );
            }

            sortCopyTieTeams( sorted_teams, tmp_teams ); // recursive operation
            sortCopyTieTeams( sorted_teams, teams ); // recursive operation
            return;
        }
    }

    //
    // Rule.6: check number of goal scored including only games with the tied teams
    //
    if ( teams.size() > 2 )
    {
        updateTiedGroupGoals( teams );
        teams.sort( TiedGoalScoredCmp() );

        if ( teams.front().tied_goal_scored_ != teams.back().tied_goal_scored_ )
        {
            std::list< Team > tmp_teams;
            tmp_teams.splice( tmp_teams.end(), teams, teams.begin() );

            while ( ! teams.empty() )
            {
                if ( teams.front().tied_goal_scored_ != tmp_teams.back().tied_goal_scored_ )
                {
                    break;
                }

                tmp_teams.splice( tmp_teams.end(), teams, teams.begin() );
            }

            sortCopyTieTeams( sorted_teams, tmp_teams ); // recursive operation
            sortCopyTieTeams( sorted_teams, teams ); // recursive operation
            return;
        }
    }

    //
    // Rule 7: same standing -> penalty shootouts
    //
    if ( ! teams.empty() )
    {
        std::cerr << "exists same standing teams:\n";
        teams.sort( TeamNameCmp() );
        for ( Team & t : teams )
        {
            std::cerr << "  " << t.name_ << '\n';
            t.tie_ = true;
        }
        std::cerr << std::endl;
        sorted_teams.splice( sorted_teams.end(), teams );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
TablePrinter::updateTiedGroupGoals( std::list< Team > & teams )
{
    for ( Team & t1 : teams )
    {
        t1.tied_goal_scored_ = 0;
        t1.tied_goal_conceded_ = 0;

        for ( Team & t2 : teams )
        {
            if ( &t1 == &t2 ) continue;

            MatchTable::const_iterator m = M_match_table.find( Match::Key( t1.name_, t2.name_ ) );
            if ( m == M_match_table.end() ) continue;

            t1.tied_goal_scored_ += m->second.score_l_;
            t1.tied_goal_conceded_ += m->second.score_r_;
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
TablePrinter::printPukiWiki( std::ostream & os ) const
{
    //
    // print standing table
    //


    os << "(ranked by (avg. points -> direct comparison -> avg.goal diff -> avg.goals scored)\n";

    os << "| place "
       << "| team             "
       << "| games "
       << "| win | lose | draw "
       << "| pts. (avg)  "
       << "| goals     "
       << "| goal diff (avg) "
       << "| avg goal scored |h\n";

    int rank_count = 0;
    bool last_tie = false;
    for ( const Team & t1 : M_teams )
    {
        ++rank_count;

        int rank = rank_count;
        if ( last_tie )
        {
            if ( t1.tie_ )
            {
                --rank;
            }
            else
            {
                last_tie = false;
            }
        }
        else
        {
            last_tie = t1.tie_;
        }

        os << "|    " << std::setw( 2 ) << std::right << rank;
        os << " | " << t1.name_;
        for ( size_t len = t1.name_.length(); len < 16; ++len )
        {
            os.put( ' ' );
        }

        // games (w,l,d)
        os << " | " << std::setw( 5 ) << std::right << t1.games_;
        os << " | " << std::setw( 3 ) << t1.win_
           << " | " << std::setw( 4 ) << t1.lose_
           << " | " << std::setw( 4 ) << t1.draw_;
        // points (avg)
        os << " | " << std::setw( 3 ) << std::right << t1.points_
           << " ("
           << std::setw( 5 ) << std::setprecision( 3 ) << std::right
           << static_cast< double >( t1.points_ )
            / static_cast< double >( t1.games_ )
           << ")";
        // goals
        os << " | " << std::setw( 3 ) << std::right << t1.goal_scored_
           << " - " << std::setw( 3 ) << std::right << t1.goal_conceded_;
        // goal diff (avg)
        os << " |    "
           << std::setw( 4 ) << std::right << t1.goal_scored_ - t1.goal_conceded_
           << " ("
           <<  std::setw( 5 ) << std::setprecision( 3 ) << std::right
           << static_cast< double >( t1.goal_scored_ - t1.goal_conceded_ )
            / static_cast< double >( t1.games_ )
           << ")";
        // avg goal scored
        os << " |           "
           <<  std::setw( 5 ) << std::setprecision( 3 ) << std::right
           << static_cast< double >( t1.goal_scored_ )
            / static_cast< double >( t1.games_ )
           << " |";
        os << '\n';
    }

    os << '\n';

    //
    // print match table
    //

    os << "|                 ";
    for ( const Team & t : M_teams )
    {
        os << " | ";
        if ( t.name_.length() > 4 )
        {
            os << t.name_.substr( 0, 3 ) << '.';
        }
        else
        {
            os << t.name_;
            for ( size_t len = t.name_.length(); len < 4; ++len )
            {
                os.put( ' ' );
            }
        }
        os << "  ";
    }
    os << " |h\n";

    for ( const Team & t1 : M_teams )
    {
        //bool upper = false;
        os << "| " << t1.name_;
        for ( size_t len = t1.name_.length(); len < 16; ++len )
        {
            os.put( ' ' );
        }

        for ( const Team & t2 : M_teams )
        {
            if ( &t1 == &t2 )
            {
                os << " |   x   ";
                //upper = true;
            }
            else
            {
                MatchTable::const_iterator m = M_match_table.find( Match::Key( t1.name_, t2.name_ ) );
                if ( m != M_match_table.end() )
                {
                    char buf[64];
                    snprintf( buf, 64, "%d - %d", m->second.score_l_, m->second.score_r_ );

                    os << " | " << buf;
                    //if ( upper ) os << 'g';
                    for ( size_t len = std::strlen( buf ); len < 6; ++len )
                    {
                        os.put( ' ' );
                    }
                }
                else
                {
                    os << " |  ---  ";
                }
            }
        }
        os << " |\n";
    }

    os << '\n';

    //
    // print match list
    //

    os << "|   # | date         | left team        | goals   | right team       "
       << "|h\n";
    int count = 0;
    for ( const Match & m : M_match_list )
    {
        ++count;
        os << "| " << std::setw( 3 ) << count;
        os << " | " << m.date_;
        os << " | " << m.name_l_;
        for ( size_t len = m.name_l_.length(); len < 16; ++len )
        {
            os.put( ' ' );
        }

        os << " | " << std::setw( 2 ) << m.score_l_
           << " - " << std::setw( 2 ) << m.score_r_;
        if ( m.pen_score_l_ + m.pen_score_r_ > 0 )
        {
            os << " &br; (" << m.pen_score_l_ << " - " << m.pen_score_r_ << ")";
        }

        os << " | " << m.name_r_;
        for ( size_t len = m.name_r_.length(); len < 16; ++len )
        {
            os.put( ' ' );
        }

        os << " |\n";
    }

    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
TablePrinter::printHtml( std::ostream & os ) const
{
    //
    // print html header
    //
    os << "<html>\n"
       << "<head>\n"
       << "<title>Results:" << M_group_name << "</title>\n"
       << "<link type=\"text/css\" rel=\"stylesheet\" href=\"./style.css\">\n"
        //<< "<link type=\"text/css\" rel=\"stylesheet\" href=\"./quali.css\">"
       << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n"
       << "</head>\n"
       << "<body>\n";

    os << "<h1 class=\"title\">" << M_group_name << "</h1>\n"
       << '\n';

    //
    // print standing table
    //

    os << "<hr />\n";

    os << "<h2 class=\"tabletitle\">Standing</h2>\n"
       << '\n';

    os << "<div class=\"sortdesc\">"
       << "(ranked by (avg. points -> direct comparison -> avg.goal diff -> avg.goals scored)"
       << "</div>\n";

    os << "<table class=\"standing\">\n";
    os << "  <tr class=\"tableheader\">\n"
       << "    <th>place</th>\n"
       << "    <th>team</th>\n"
       << "    <th>games</th>\n"
       << "    <th>win</th>\n"
       << "    <th>lose</th>\n"
       << "    <th>draw</th>\n"
       << "    <th>pts. (avg)</th>\n"
       << "    <th>goals</th>\n"
       << "    <th>goal diff (avg)</th>\n"
       << "    <th>avg goals scored</th>\n"
       << "  </tr>\n";

    int rank_count = 0;
    bool last_tie = false;
    for ( const Team & t : M_teams )
    {
        ++rank_count;
        int rank = rank_count;
        if ( last_tie )
        {
            if ( t.tie_ )
            {
                --rank;
            }
            else
            {
                last_tie = false;
            }
        }
        else
        {
            last_tie = t.tie_;
        }

        os << "  <tr class=\"";
        if ( rank <= 3 ) os << "place" << rank;
        else if ( rank % 2 == 0 ) os << "even";
        else  os << "odd";
        os << "\">\n";

        os << "    <td>" << rank << "</td>\n";
        os << "    <th>" << t.name_ << "</th>\n";
        os << "    <td>" << t.games_ << "</td>\n";
        os << "    <td>" << t.win_ << "</td>\n";
        os << "    <td>" << t.lose_ << "</td>\n";
        os << "    <td>" << t.draw_ << "</td>\n";
        // points (avg)
        os << "    <td>" << t.points_
           << " (" << std::setprecision( 3 )
           << static_cast< double >( t.points_ )
            / static_cast< double >( t.games_ )
           << ")</td>\n";
        // goals
        os << "    <td>" << t.goal_scored_ << " - " << t.goal_conceded_ << "</td>\n";
        // goal diff (avg)
        os << "    <td>" << t.goal_scored_ - t.goal_conceded_
           << " (" << std::setprecision( 3 )
           << static_cast< double >( t.goal_scored_ - t.goal_conceded_ )
            / static_cast< double >( t.games_ )
           << ")</td>\n";
        // avg goal scored
        os << "    <td>" << std::setprecision( 3 )
           << static_cast< double >( t.goal_scored_ )
            / static_cast< double >( t.games_ )
           << "</td>\n";

        os << "  </tr>\n"
           << '\n';
    }

    os << "</table>\n";

    //
    // print match table
    //


    os << "<hr />\n";

    os << "<h2 class=\"tabletitle\">Results</h2>\n"
       << '\n';

    os << "<table class=\"resultstable\">\n";
    os << "  <tr class=\"tableheader\">\n";
    os << "    <th>&nbsp;</th>\n";
    for ( const Team & t : M_teams )
    {
        os << "    <th>";
        if ( t.name_.length() > 4 )
        {
            os << t.name_.substr( 0, 3 ) << '.';
        }
        else
        {
            os << t.name_;
        }
        os << "</th>\n";
    }
    os << "  </tr>\n";

    int count = 0;
    for ( const Team & t1 : M_teams )
    {
        ++count;
        os << "  <tr class=\"";
        if ( count % 2 == 0 ) os << "even";
        else os << "odd";
        os << "\">\n";

        bool upper = false;
        os << "    <th>" << t1.name_ << "</th>\n";

        for ( const Team & t2 : M_teams )
        {
            os << "    <td>";
            if ( &t1 == &t2 )
            {
                os << " x ";
                upper = true;
            }
            else
            {
                MatchTable::const_iterator m = M_match_table.find( Match::Key( t1.name_, t2.name_ ) );
                if ( m != M_match_table.end() )
                {
                    char basename[256];
                    if ( m->second.hasPenaltyScore() )
                    {
                        snprintf( basename, 256, "%s-%s_%d_%d-vs-%s_%d_%d",
                                  m->second.date_.c_str(),
                                  m->second.realLeftTeamName().c_str(),
                                  m->second.realLeftScore(),
                                  m->second.realLeftPenScore(),
                                  m->second.realRightTeamName().c_str(),
                                  m->second.realRightScore(),
                                  m->second.realRightPenScore() );
                    }
                    else
                    {
                        snprintf( basename, 256, "%s-%s_%d-vs-%s_%d",
                                  m->second.date_.c_str(),
                                  m->second.realLeftTeamName().c_str(),
                                  m->second.realLeftScore(),
                                  m->second.realRightTeamName().c_str(),
                                  m->second.realRightScore() );
                    }

                    os << "<a href=\"" << M_log_dir << basename;

                    if ( upper ) os << ".rcg.gz";
                    else os << ".rcl.gz";
                    os << "\">";

                    os << m->second.score_l_ << " - " << m->second.score_r_;
                    os << "</a>";
                }
                else
                {
                    os << " --- ";
                }
            }
            os << "</td>\n";
        }
        os << "  </tr>\n";
    }

    os << "</table>\n"
       << '\n';


    //
    // print match list
    //

    os << "<table class=\"matchlist\">\n";
    os << "  <tr>\n"
       << "    <th> # </th>\n"
       << "    <th> date </th>\n"
       << "    <th> vs </th>\n"
       << "    <th> goals </th>\n"
       << "    <th> points </th>\n"
       << "    <th> rcl </th>\n"
       << "    <th> rcg </th>\n"
       << "  </tr>\n";

    count = 0;
    for ( const Match & m : M_match_list )
    {
        ++count;
        os << "  <tr class=\"";
        if ( count % 2 == 0 ) os << "even";
        else os << "odd";
        os << "\">\n";

        int point_l = calcPoint( m.score_l_, m.pen_score_l_, m.score_r_, m.pen_score_r_ );
        int point_r = point_l == 3 ? 0 : point_l == 0 ? 3 : 1;

        os << "    <td>" << count << "</td>\n";
        os << "    <td>" << m.date_ << "</td>\n";
        os << "    <td>";
        if ( point_l > point_r ) os << "<span class=\"winner\">" << m.name_l_ << "</span>";
        else os << m.name_l_;
        os << " vs ";
        if ( point_r > point_l ) os << "<span class=\"winner\">" << m.name_r_ << "</span>";
        else os << m.name_r_;
        os<< "</td>\n";
        os << "    <td>" << m.score_l_ << " : " << m.score_r_;
        if ( m.hasPenaltyScore() )
        {
            os << " <br />(" << m.pen_score_l_ << " : " << m.pen_score_r_ << ")";
        }
        os << "</td>\n";
        os << "    <td>" << point_l << " : " << point_r << "</td>\n";

        char basename[256];

        if ( m.hasPenaltyScore() )
        {
            snprintf( basename, 256, "%s-%s_%d_%d-vs-%s_%d_%d",
                      m.date_.c_str(),
                      m.realLeftTeamName().c_str(),
                      m.realLeftScore(),
                      m.realLeftPenScore(),
                      m.realRightTeamName().c_str(),
                      m.realRightScore(),
                      m.realRightPenScore() );
        }
        else
        {
            snprintf( basename, 256, "%s-%s_%d-vs-%s_%d",
                      m.date_.c_str(),
                      m.realLeftTeamName().c_str(),
                      m.realLeftScore(),
                      m.realRightTeamName().c_str(),
                      m.realRightScore() );
        }

        os << "    <td><a href=\"" << M_log_dir << basename << ".rcg.gz\">"
           << "rcg</a></td>\n";

        os << "    <td><a href=\"" << M_log_dir << basename << ".rcl.gz\">"
           << "rcl</a></td>\n";

        os << "  </tr>\n";
    }


    os << "</table>\n"
       << '\n';

    //
    // print html footer
    //

    os << "<hr />\n"
       << '\n';
    os << "<body>\n"
       << "</html>\n";

    os << std::flush;
    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
TablePrinter::printXml( std::ostream & os ) const
{
    std::cerr << "XML format is not supported yet." << std::endl;
    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
TablePrinter::print() const
{
    std::ostream * os = &std::cout;

    if ( ! M_output_file.empty()
         && M_output_file != "-" )
    {
        os = new std::ofstream( M_output_file.c_str() );
        if ( ! *os )
        {
            std::cerr << "Error: could not open the output file. " << M_output_file
                      << std::endl;
            delete os;
            return false;
        }
    }

    switch ( M_print_type ) {
    case PukiWiki:
        printPukiWiki( *os );
        break;
    case HTML:
        printHtml( *os );
        break;
    case XML:
        printXml( *os );
        break;
    default:
        break;
    }

    if ( os != &std::cout )
    {
        os->flush();
        delete os;
        os = &std::cout;
    }

    return true;
}


////////////////////////////////////////////////////////////////////////

int
main( int argc, char ** argv )
{
    TablePrinter printer;

    if ( ! printer.parseCmdLine( argc, argv ) )
    {
        return 1;
    }

    if ( ! printer.read() )
    {
        return 1;
    }

    printer.print();

    return 0;
}
