// -*-c++-*-

/*!
  \file rcgrenameteam.cpp
  \brief team name renamer in the rcg file source File.
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

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include <rcsc/gz.h>
#include <rcsc/rcg.h>

#include <memory>
#include <cmath>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>

class TeamNameRenamer
    : public rcsc::rcg::Handler {
private:

    std::ostream & M_os;

    rcsc::rcg::Serializer::Ptr M_serializer;

    std::string M_left_team_name;
    std::string M_right_team_name;

    // not used
    TeamNameRenamer() = delete;
public:

    TeamNameRenamer( std::ostream & os,
                     const std::string & left_team_name,
                     const std::string & right_team_name );

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
};


/*-------------------------------------------------------------------*/
/*!

*/
TeamNameRenamer::TeamNameRenamer( std::ostream & os,
                                  const std::string & left_team_name,
                                  const std::string & right_team_name )
    : M_os( os )
    , M_left_team_name( left_team_name )
    , M_right_team_name( right_team_name )
{
    M_serializer = rcsc::rcg::Serializer::create( 1 );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TeamNameRenamer::handleLogVersion( const int ver )
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
TeamNameRenamer::handleEOF()
{
    M_os.flush();
    return true;
}


/*-------------------------------------------------------------------*/
/*!

 */
bool
TeamNameRenamer::handleShow( const rcsc::rcg::ShowInfoT & show )
{
    if ( ! M_serializer )
    {
        return false;
    }

    M_serializer->serialize( M_os, show );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
TeamNameRenamer::handleMsg( const int,
                            const int board,
                            const std::string & msg )
{
    if ( ! M_serializer )
    {
        return false;
    }

    M_serializer->serialize( M_os, board, std::string( msg ) );

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
TeamNameRenamer::handleDraw( const int,
                             const rcsc::rcg::drawinfo_t & draw )
{
    if ( ! M_serializer )
    {
        return false;
    }

    M_serializer->serialize( M_os, draw );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
TeamNameRenamer::handlePlayMode( const int,
                                 const rcsc::PlayMode pm )
{
    if ( ! M_serializer )
    {
        return false;
    }

    M_serializer->serialize( M_os, static_cast< char >( pm ) );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
TeamNameRenamer::handleTeam( const int,
                             const rcsc::rcg::TeamT & team_l,
                             const rcsc::rcg::TeamT & team_r )
{
    if ( ! M_serializer )
    {
        return false;
    }

    rcsc::rcg::TeamT teams[2];
    teams[0] = team_l;
    teams[1] = team_r;

    if ( ! M_left_team_name.empty() )
    {
        teams[0].name_ = M_left_team_name;
    }

    if ( ! M_right_team_name.empty() )
    {
        teams[1].name_ = M_right_team_name;
    }

    M_serializer->serialize( M_os, teams[0], teams[1] );

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
TeamNameRenamer::handleServerParam( const std::string & msg )
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
TeamNameRenamer::handlePlayerParam( const std::string & msg )
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
TeamNameRenamer::handlePlayerType( const std::string & msg )
{
   if ( ! M_serializer )
    {
        return false;
    }

    M_serializer->serializeParam( M_os, msg );
    return true;
}


///////////////////////////////////////////////////////////

/*---------------------------------------------------------------*/
/*
  Usage:
  $ rcgrenameteam <RCGFile>[.gz]
*/
void
usage( const char * prog )
{
    std::cerr << "Usage: " << prog <<  " [Options] <RcgFile>[.gz] -o <OutputFile>\n"
              << "Available options:\n"
              << "    --help [ -h ]\n"
              << "        print this message.\n"
              << "    --left <Value> : (DefaultValue=\"\")\n"
              << "        specify the left team name.\n"
              << "    --right <Value> : (DefaultValue=\"\")\n"
              << "        specify the right team name.\n"
              << "    --output [ -o ]<Value>\n"
              << "        specify the output file name.\n"
              << std::endl;
}


////////////////////////////////////////////////////////////////////////

int
main( int argc, char** argv )
{
    std::string input_file;
    std::string output_file;
    std::string left_team_name;
    std::string right_team_name;

    for ( int i = 1; i < argc; ++i )
    {
        if ( ! std::strcmp( argv[i], "--help" )
             || ! std::strcmp( argv[i], "-h" ) )
        {
            usage( argv[0] );
            return 0;
        }
        else if ( ! std::strcmp( argv[i], "--left" ) )
        {
            ++i;
            if ( i >= argc )
            {
                usage( argv[0] );
                return 1;
            }
            left_team_name = argv[i];
        }
        else if ( ! std::strcmp( argv[i], "--right" ) )
        {
            ++i;
            if ( i >= argc )
            {
                usage( argv[0] );
                return 1;
            }
            right_team_name = argv[i];
        }
        else if ( ! std::strcmp( argv[i], "--output" )
                  || ! std::strcmp( argv[i], "-o" ) )
        {

            ++i;
            if ( i >= argc )
            {
                usage( argv[0] );
                return 1;
            }
            output_file = argv[i];
        }
        else
        {
            input_file = argv[i];
        }
    }

    if ( input_file.empty() )
    {
        std::cerr << "No input file" << std::endl;
        usage( argv[0] );
        return 1;
    }

    if ( output_file.empty() )
    {
        std::cerr << "No output file" << std::endl;
        usage( argv[0] );
        return 1;
    }

    if ( input_file == output_file )
    {
        std::cerr << "The output file is same as the input file." << std::endl;
        return 1;
    }

    if ( left_team_name.empty()
         && right_team_name.empty() )
    {
        std::cerr << "No new team names!" << std::endl;
        usage( argv[0] );
        return 1;
    }

    if ( left_team_name == right_team_name )
    {
        std::cerr << "Same team names!" << std::endl;
        return 1;
    }

    rcsc::gzifstream fin( input_file.c_str() );

    if ( ! fin.is_open() )
    {
        std::cerr << "Failed to open file : " << input_file << std::endl;
        return 1;
    }

    std::shared_ptr< std::ostream > fout;

    if ( output_file.compare( output_file.length() - 3, 3, ".gz" ) == 0 )
    {
        fout = std::shared_ptr< std::ostream >( new rcsc::gzofstream( output_file.c_str() ) );
    }
    else
    {
        fout = std::shared_ptr< std::ostream >( new std::ofstream( output_file.c_str(),
                                                                   std::ios_base::out | std::ios_base::binary ) );
    }

    if ( ! fout
         || fout->fail() )
    {
        std::cerr << "output stream for the new rcg file. [" << output_file
                  << "] is not good." << std::endl;
        return 1;
    }

    rcsc::rcg::Parser::Ptr parser = rcsc::rcg::Parser::create( fin );

    if ( ! parser )
    {
        std::cerr << "Failed to create rcg parser." << std::endl;
        return 1;
    }

    // create rcg handler instance
    TeamNameRenamer renamer( *fout,
                             left_team_name,
                             right_team_name );

    parser->parse( fin, renamer );

    return 0;
}
