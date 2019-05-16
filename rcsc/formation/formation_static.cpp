// -*-c++-*-

/*!
  \file formation_static.cpp
  \brief static type formation method classes Source File.
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

#include "formation_static.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <cstdio>

namespace rcsc {

using namespace formation;

const std::string FormationStatic::NAME( "Static" );

/*-------------------------------------------------------------------*/
/*!

*/
FormationStatic::FormationStatic()
    : Formation()
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
FormationStatic::createDefaultData()
{
    createNewRole( 1, "Goalie", Formation::CENTER );
    createNewRole( 2, "CenterBack", Formation::SIDE );
    setSymmetryType( 3, 2, "CenterBack" );
    createNewRole( 4, "SideBack", Formation::SIDE );
    setSymmetryType( 5, 4, "SideBack" );
    createNewRole( 6, "DefensiveHalf", Formation::CENTER );
    createNewRole( 7, "OffensiveHalf", Formation::SIDE );
    setSymmetryType( 8, 7, "OffensiveHalf" );
    createNewRole( 9, "SideForward", Formation::SIDE );
    setSymmetryType( 10, 9, "SideForward" );
    createNewRole( 11, "CenterForward", Formation::CENTER );

    SampleData data;

    data.ball_.assign( 0.0, 0.0 );
    data.players_.push_back( Vector2D( -50.0, 0.0 ) );
    data.players_.push_back( Vector2D( -20.0, -8.0 ) );
    data.players_.push_back( Vector2D( -20.0, 8.0 ) );
    data.players_.push_back( Vector2D( -18.0, -18.0 ) );
    data.players_.push_back( Vector2D( -18.0, 18.0 ) );
    data.players_.push_back( Vector2D( -15.0, 0.0 ) );
    data.players_.push_back( Vector2D( 0.0, -12.0 ) );
    data.players_.push_back( Vector2D( 0.0, 12.0 ) );
    data.players_.push_back( Vector2D( 10.0, -22.0 ) );
    data.players_.push_back( Vector2D( 10.0, 22.0 ) );
    data.players_.push_back( Vector2D( 10.0, 0.0 ) );

    M_samples->addData( *this, data, false );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FormationStatic::createNewRole( const int unum,
                                const std::string & role_name,
                                const SideType type )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** invalid unum " << unum
                  << std::endl;
        return;
    }

    setRoleName( unum, role_name );

    switch ( type ) {
    case Formation::CENTER:
        setCenterType( unum );
        break;
    case Formation::SIDE:
        setSideType( unum );
        break;
    case Formation::SYMMETRY:
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** Invalid side type "
                  << std::endl;
        break;
    default:
        break;
    }

}

/*-------------------------------------------------------------------*/
/*!

*/
void
FormationStatic::setRoleName( const int unum,
                              const std::string & name )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ***ERROR*** invalid unum " << unum
                  << std::endl;
        return;
    }

    M_role_names[unum - 1] = name;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::string
FormationStatic::getRoleName( const int unum ) const
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ***ERROR*** Invalid unum"
                  << std::endl;
        return std::string( "null" );
    }

    return M_role_names[unum - 1];
}

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
FormationStatic::getPosition( const int unum,
                              const Vector2D & ) const
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " **ERROR*** Invalid player number " << unum
                  << std::endl;
        return Vector2D( 0.0, 0.0 );
    }

    return M_pos[unum - 1];
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FormationStatic::getPositions( const Vector2D & focus_point,
                               std::vector< Vector2D > & positions ) const
{
    positions.clear();

    for ( int unum = 1; unum <= 11; ++unum )
    {
        positions.push_back( getPosition( unum, focus_point ) );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FormationStatic::train()
{
    //std::cerr << "Static method never support any training" << std::endl;

}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationStatic::read( std::istream & is )
{
    //return readOld( is );
    return readCSV( is );
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationStatic::readOld( std::istream & is )
{
    if ( ! readHeader( is ) ) return false;
    if ( ! readConf( is ) ) return false;
    if ( ! readSamplesOld( is ) ) return false;

    if ( ! checkSymmetryNumber() )
    {
        std::cerr << __FILE__ << " *** ERROR *** Illegal symmetry data."
                  << std::endl;
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationStatic::print( std::ostream & os ) const
{
    return printCSV( os );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationStatic::printOld( std::ostream & os ) const
{
    if ( os ) printHeader( os );
    if ( os ) printConf( os );

    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
FormationStatic::readConf( std::istream & is )
{
    if ( ! readPlayers( is ) )
    {
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
FormationStatic::readPlayers( std::istream & is )
{
    int n_read = 0;

    std::string line_buf;

    for ( int i = 0; i < 11; ++i )
    {
        while ( std::getline( is, line_buf ) )
        {
            if ( line_buf.empty()
                 || line_buf[0] == '#'
                 || ! line_buf.compare( 0, 2, "//" ) )
            {
                continue;
            }

            int unum;
            char role_name[128];
            double pos_x, pos_y;

            if ( std::sscanf( line_buf.c_str(),
                              " %d %s %lf %lf ",
                              &unum, role_name, &pos_x, &pos_y ) != 4 )
            {
                continue;
            }

            if ( unum != i + 1 )
            {
                std::cerr << __FILE__ << ':' << __LINE__
                          << " ***ERROR*** Invalid formation formart ["
                          << line_buf << "]"
                          << std::endl;
                return false;
            }

            M_pos[i].assign( pos_x, pos_y );
            M_role_names[i] = role_name;
            ++n_read;
            break;
        }
    }

    if ( n_read != 11 )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ***ERROR*** Invalid formation format."
                  << " The number of read player is " << n_read
                  << std::endl;
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
FormationStatic::printConf( std::ostream & os ) const
{
    for ( int i = 0; i < 11; ++i )
    {
        os << i + 1 << ' '
           << M_role_names[i] << ' '
           << M_pos[i].x << ' '
           << M_pos[i].y << '\n';
    }

    return os << std::flush;
}


/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationStatic::readCSV( std::istream & is )
{
    std::vector< int > role_numbers;
    std::vector< std::string > role_names;

    if ( ! readMethodName( is ) ) return false;

    std::string line_buf;
    std::vector< std::string > values;

    while ( std::getline( is, line_buf ) )
    {
        if ( line_buf.empty()
             || line_buf[0] == '#' )
        {
            continue;
        }

        boost::split( values, line_buf, boost::is_any_of( " ,") );

        if ( values.empty() )
        {
            std::cerr << __FILE__ << " (readCSV) ERROR Empty line."
                      << std::endl;
            return false;
        }

        if ( values.front() == "RoleNumber" )
        {
            if ( values.size() != 12 )
            {
                std::cerr << __FILE__ << " (readCSV) ERROR Illegal # of role numberss." << std::endl;
                return false;
            }

            for ( int i = 1; i <= 11; ++i )
            {
                try
                {
                    role_numbers.push_back( boost::lexical_cast< int >( values[i] ) );
                }
                catch ( const boost::bad_lexical_cast & e )
                {
                    std::cerr << __FILE__ << " (readCSV) ERROR read role numbers. " << e.what() << std::endl;
                    return false;
                }
            }
        }
        else if ( values.front() == "RoleName" )
        {
            role_names.assign( values.begin() + 1, values.end() );
            if ( role_names.size() != 11 )
            {
                std::cerr << __FILE__ << " (readCSV) ERROR Illegal # of role names." << std::endl;
                return false;
            }
            std::copy( role_names.begin(), role_names.end(), M_role_names );
        }
        else if ( values.front() == "Position" )
        {
            if ( values.size() != 1 + 11*2 )
            {
                std::cerr << __FILE__ << " (readCSV) ERROR Illegal # of position values." << std::endl;
                return false;
            }

            for ( int i = 0; i < 11; ++i )
            {
                try
                {
                    M_pos[i].x = boost::lexical_cast< double >( values[ i*2 + 1 ] );
                    M_pos[i].y = boost::lexical_cast< double >( values[ i*2 + 2 ] );
                }
                catch ( const boost::bad_lexical_cast & e )
                {
                    std::cerr << __FILE__ << " (readCSV) ERROR read position valuess. " << e.what() << std::endl;
                    return false;
                }
            }

        }
        else
        {
            std::cerr << __FILE__ << " (readCSV) Unsupported data line. [" << values.front() << ']' << std::endl;
            return false;
        }

        values.clear();
    }

    if ( role_numbers.size() != 11
         || role_names.size() != 11 )
    {
        std::cerr << __FILE__ << " (readCSV) ERROR Illegal # of data." << std::endl;
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationStatic::printCSV( std::ostream & os ) const
{
    printMethodName( os );
    printRoleNumbers( os );
    printRoleNames( os );
    printPositions( os );

    return os;
}


std::ostream &
FormationStatic::printRoleNumbers( std::ostream & os ) const
{
    os << "RoleNumber";
    for ( int i = 1; i <= 11; ++i )
    {
        os << ',' << i;
    }
    os << '\n';

    return os;
}

std::ostream &
FormationStatic::printRoleNames( std::ostream & os ) const
{
    os << "RoleName";
    for ( int i = 0; i < 11; ++i )
    {
        os << ',' << M_role_names[i];
    }
    os << '\n';
    return os;
}


std::ostream &
FormationStatic::printPositions( std::ostream & os ) const
{
    os << "#PositionIdx";
    for ( int i = 1; i <= 11; ++i )
    {
        os << ',' << i << 'X'
           << ',' << i << 'Y';
    }
    os << '\n';

    os << "Position";
    for ( int i = 0; i < 11; ++i )
    {
        os << ',' << M_pos[i].x << ',' << M_pos[i].y;
    }
    os << '\n';
    return os;
}




/*-------------------------------------------------------------------*/
/*!

*/
namespace {

Formation::Ptr
create()
{
    Formation::Ptr ptr( new FormationStatic() );
    return ptr;
}

rcss::RegHolder f = Formation::creators().autoReg( &create,
                                                   FormationStatic::NAME );

}

}
