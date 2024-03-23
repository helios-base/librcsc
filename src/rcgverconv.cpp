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

#include <rcsc/gz.h>
#include <rcsc/rcg.h>

#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <cstring>
#include <cstdlib>

class VersionConverter
    : public rcsc::rcg::Handler {
private:

    std::ostream & M_os;
    int M_target_rcg_version;

    rcsc::rcg::Serializer::Ptr M_serializer;

    // not used
    VersionConverter() = delete;
public:

    VersionConverter( std::ostream & os,
                      const int version );

    bool handleLogVersion( const int ver ) override;

    bool handleEOF() override;

    bool handleShow( const rcsc::rcg::ShowInfoT & show );
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
    bool handleTeamGraphic( const char side,
                            const int x,
                            const int y,
                            const std::vector< std::string > & xpm ) override;
};


/*-------------------------------------------------------------------*/
/*!

*/
VersionConverter::VersionConverter( std::ostream & os,
                                    const int version )
    : M_os( os ),
      M_target_rcg_version( version )
{
    M_serializer = rcsc::rcg::Serializer::create( version );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
VersionConverter::handleLogVersion( const int ver )
{
    rcsc::rcg::Handler::handleLogVersion( ver );

    if ( ver == M_target_rcg_version )
    {
        std::cerr << "The version of input file (" << ver
                  << ") is same as the output version ("
                  << M_target_rcg_version << ")"
                  << std::endl;
        M_serializer.reset();
        return false;
    }

    if ( ! M_serializer )
    {
        std::cerr << "No serializer!\n"
                  << "Unsupported rcg version may be specified."
                  << std::endl;
        return false;
    }

    M_serializer->serializeBegin( M_os, serverVersion(), timestamp() );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
VersionConverter::handleEOF()
{
    if ( M_serializer )
    {
        M_serializer->serializeEnd( M_os );
    }

    M_os.flush();
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
VersionConverter::handleShow( const rcsc::rcg::ShowInfoT & show )
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
VersionConverter::handleMsg( const int,
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
VersionConverter::handleDraw( const int,
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
VersionConverter::handlePlayMode( const int,
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
VersionConverter::handleTeam( const int,
                              const rcsc::rcg::TeamT & team_l,
                              const rcsc::rcg::TeamT & team_r )
{
    if ( ! M_serializer )
    {
        return false;
    }

    M_serializer->serialize( M_os, team_l, team_r );
    return true;
}

/*-------------------------------------------------------------------*/
bool
VersionConverter::handleServerParam( const rcsc::rcg::ServerParamT & param )
{
    if ( ! M_serializer )
    {
        return false;
    }

    M_serializer->serialize( M_os, param );
    return true;
}

/*-------------------------------------------------------------------*/
bool
VersionConverter::handlePlayerParam( const rcsc::rcg::PlayerParamT & param )
{
    if ( ! M_serializer )
    {
        return false;
    }

    M_serializer->serialize( M_os, param );
    return true;
}

/*-------------------------------------------------------------------*/
bool
VersionConverter::handlePlayerType( const rcsc::rcg::PlayerTypeT & param )
{
    if ( ! M_serializer )
    {
        return false;
    }

    M_serializer->serialize( M_os, param );
    return true;
}

bool
VersionConverter::handleTeamGraphic( const char side,
                                     const int x,
                                     const int y,
                                     const std::vector< std::string > & xpm )
{
    if ( ! M_serializer )
    {
        return false;
    }

    M_serializer->serialize( M_os, side, x, y, xpm );
    return true;
}

///////////////////////////////////////////////////////////

/*---------------------------------------------------------------*/
/*

*/
static
void
usage( const char * prog )
{
    std::cerr << "Usage: " << prog <<  " [Options] <RcgFile>[.gz] -o <OutputFile>\n"
              << "Available options:\n"
              << "    --help [ -h ]\n"
              << "        print this message.\n"
              << "    --version [ -v ] <Value> : (DefaultValue=json)\n"
              << "        specify the new rcg version.\n"
              << "    --output [ -o ] <Value>\n"
              << "        specify the output file name.\n"
              << std::endl;
}


////////////////////////////////////////////////////////////////////////

int
main( int argc, char** argv )
{
    std::string input_file;
    std::string output_file;
    int version = -1;

    for ( int i = 1; i < argc; ++i )
    {
        if ( ! std::strcmp( argv[i], "--help" )
             || ! std::strcmp( argv[i], "-h" ) )
        {
            usage( argv[0] );
            return 0;
        }
        else if ( ! std::strcmp( argv[i], "--version" )
                  || ! std::strcmp( argv[i], "-v" ) )
        {
            ++i;
            if ( i >= argc )
            {
                usage( argv[0] );
                return 1;
            }

            if ( std::strncmp( argv[i], "json", 4 ) == 0 )
            {
                version = -1;
            }
            else
            {
                version = std::atoi( argv[i] );
            }
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

    if ( version == 0 )
    {
        std::cerr << "Unsupported game log version = " << version << std::endl;
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
    VersionConverter converter( *fout, version );

    parser->parse( fin, converter );

    return 0;
}
