// -*-c++-*-

/*!
  \file formation_bpn.cpp
  \brief BPN formation data classes Source File.
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

#include "formation_bpn.h"

#include <rcsc/math_util.h>

#include <random>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <cstdio>

namespace rcsc {

using namespace formation;

const std::string FormationBPN::NAME( "BPN" );

const double FormationBPN::Param::PITCH_LENGTH = 105.0 + 10.0;
const double FormationBPN::Param::PITCH_WIDTH = 68.0 + 10.0;


/*-------------------------------------------------------------------*/
/*!

*/
FormationBPN::Param::Param()
    : M_net( 0.3, 0.9 )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
FormationBPN::Param::randomize()
{
    static std::mt19937 s_engine( std::time( 0 ) );
    std::uniform_real_distribution<> dst( -0.5, 0.5 );

    M_net.randomize( [&](){ return dst( s_engine ); } );
}

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
FormationBPN::Param::getPosition( const Vector2D & ball_pos,
                                  const Formation::SideType type ) const
{
    double msign = 1.0;
    if ( type == Formation::SYMMETRY ) msign =  -1.0;
    if ( type == Formation::CENTER && ball_pos.y > 0.0 ) msign = -1.0;

    Net::input_array input;

    input[0] = std::max( 0.0,
                         std::min( ball_pos.x / PITCH_LENGTH + 0.5,
                                   1.0 ) );
    input[1] = std::max( 0.0,
                         std::min( (ball_pos.y * msign) / PITCH_WIDTH + 0.5,
                                   1.0 ) );

    Net::output_array output;

    M_net.propagate( input, output );
    //std::cerr << "getPosition. raw output = "
    //<< output[0] << " ,  " << output[1]
    //<< std::endl;
    return Vector2D( ( output[0] - 0.5 ) * PITCH_LENGTH,
                     ( output[1] - 0.5 ) * PITCH_WIDTH * msign );
}

/*-------------------------------------------------------------------*/
/*!
  Format:  Role <RoleNameStr>
*/
bool
FormationBPN::Param::readRoleName( std::istream & is )
{
    std::string line_buf;
    if ( ! std::getline( is, line_buf ) )
    {
        std::cerr  << __FILE__ << ":" << __LINE__
                   << " Failed to read" << std::endl;
        return false;
    }

    std::istringstream istr( line_buf );
    if ( ! istr.good() )
    {
        std::cerr  << __FILE__ << ":" << __LINE__
                   << " Failed to read" << std::endl;
        return false;
    }

    std::string tag;
    istr >> tag;
    if ( tag != "Role" )
    {
        std::cerr  << __FILE__ << ":" << __LINE__
                   << " Failed to read" << std::endl;
        return false;
    }

    istr >> M_role_name;
    if ( M_role_name.empty() )
    {
        std::cerr  << __FILE__ << ":" << __LINE__
                   << " Failed to read role name" << std::endl;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
FormationBPN::Param::readNet( std::istream & is )
{
    std::string line_buf;
    if ( ! std::getline( is, line_buf ) )
    {
        std::cerr  << __FILE__ << ":" << __LINE__
                   << " Failed to read" << std::endl;
        return false;
    }

    std::istringstream istr( line_buf );
    if ( ! istr.good() )
    {
        std::cerr  << __FILE__ << ":" << __LINE__
                   << " Failed to read" << std::endl;
        return false;
    }

    if ( ! M_net.read( istr ) )
    {
        std::cerr  << __FILE__ << ":" << __LINE__
                   << " Failed to read position param" << std::endl;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
FormationBPN::Param::read( std::istream & is )
{
    // read role name
    if ( ! readRoleName( is ) )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " Failed to read role name" << std::endl;
        return false;
    }

    if ( ! readNet( is ) )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " Failed to read parameters" << std::endl;
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
FormationBPN::Param::printRoleName( std::ostream & os ) const
{
    if ( M_role_name.empty() )
    {
        os << "Role Default\n";
    }
    else
    {
        os << "Role " << M_role_name << '\n';
    }
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
FormationBPN::Param::printNet( std::ostream & os ) const
{
    M_net.print( os ) << '\n';
    return os;
}

/*-------------------------------------------------------------------*/
/*
  Role <role name>
  <bko.x> <bkoy>
  <offense playon>
  <defense playon>
  ...

*/
std::ostream &
FormationBPN::Param::print( std::ostream & os ) const
{
    printRoleName( os );
    printNet( os );

    return os << std::flush;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
FormationBPN::FormationBPN()
    : Formation()
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
FormationBPN::createDefaultData()
{
    if ( ! M_samples )
    {
        M_samples = SampleDataSet::Ptr( new SampleDataSet() );
    }

    // 1: goalie
    // 2: left center back
    // 3(2): right center back
    // 4: left side back
    // 5(4): right side back
    // 6: defensive half
    // 7: left offensive half
    // 8(7): left side half
    // 9(8): right side half
    // 10: left forward
    // 11(10): right forward
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
    data.players_.emplace_back( -50.0, 0.0 );
    data.players_.emplace_back( -20.0, -8.0 );
    data.players_.emplace_back( -20.0, 8.0 );
    data.players_.emplace_back( -18.0, -18.0 );
    data.players_.emplace_back( -18.0, 18.0 );
    data.players_.emplace_back( -15.0, 0.0 );
    data.players_.emplace_back( 0.0, -12.0 );
    data.players_.emplace_back( 0.0, 12.0 );
    data.players_.emplace_back( 10.0, -22.0 );
    data.players_.emplace_back( 10.0, 22.0 );
    data.players_.emplace_back( 10.0, 0.0 );

    M_samples->addData( *this, data, false );
}


/*-------------------------------------------------------------------*/
/*!

*/
void
FormationBPN::setRoleName( const int unum,
                           const std::string & name )
{
    std::shared_ptr< Param > p = getParam( unum );

    if ( ! p )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " You cannot set the role name of player "
                  << unum
                  << std::endl;
        return;
    }

    p->setRoleName( name );
}

/*-------------------------------------------------------------------*/
/*!

*/
std::string
FormationBPN::getRoleName( const int unum ) const
{
    const std::shared_ptr< const Param > p = getParam( unum );
    if ( ! p )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " You cannot get the role name of player "
                  << unum
                  << std::endl;
        return std::string( "" );;
    }

    return p->roleName();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FormationBPN::createNewRole( const int unum,
                             const std::string & role_name,
                             const Formation::SideType type )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** invalid unum " << unum
                  << std::endl;
        return;
    }

    if ( type == Formation::CENTER )
    {
        setCenterType( unum );
    }
    else if ( type == Formation::SIDE )
    {
        setSideType( unum );
    }
    else
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** You cannot create a new parameter for symmetry type."
                  << std::endl;
        return;
    }

    // erase old parameter, if exist
    std::map< int, std::shared_ptr< Param > >::iterator it = M_param_map.find( unum );
    if ( it != M_param_map.end() )
    {
        M_param_map.erase( it );
    }

    std::shared_ptr< Param > param( new Param() );
    param->setRoleName( role_name );
    param->randomize();

    M_param_map.insert( std::make_pair( unum, param ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
FormationBPN::getPosition( const int unum,
                           const Vector2D & ball_pos ) const
{
    const std::shared_ptr< const Param > ptr = getParam( unum );
    if ( ! ptr )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " *** ERROR *** FormationBPN::Param not found. unum = "
                  << unum
                  << std::endl;
        return Vector2D::INVALIDATED;
    }

    Formation::SideType type = Formation::SIDE;
    if ( M_symmetry_number[unum - 1] > 0 )  type = Formation::SYMMETRY;
    if ( M_symmetry_number[unum - 1] == 0 ) type = Formation::CENTER;

    return ptr->getPosition( ball_pos, type );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FormationBPN::getPositions( const Vector2D & focus_point,
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
FormationBPN::train()
{
    if ( ! M_samples
         || M_samples->dataCont().empty() )
    {
        return;
    }

    const double PITCH_LENGTH = FormationBPN::Param::PITCH_LENGTH;
    const double PITCH_WIDTH = FormationBPN::Param::PITCH_WIDTH;

    std::cerr << "FormationBPN::train. Started!!" << std::endl;

    for ( int unum = 1; unum <= 11; ++unum )
    {
        int number = unum;
        Formation::SideType type = Formation::SIDE;
        if ( isSymmetryType( unum ) )
        {
            std::cerr << "  Training. player " << unum << " >>> symmetry type "
                      << std::endl;
            continue;
        }

        if ( isCenterType( unum ) )
        {
            type = Formation::CENTER;
            std::cerr << "  Training. player " << unum << " >>> center type "
                      << std::endl;
        }
        else
        {
            std::cerr << "  Training. player " << unum << " >>> side type "
                      << std::endl;
        }

        std::shared_ptr< Param > param = getParam( number );
        if ( ! param )
        {
            std::cerr << __FILE__ << ": " << __LINE__
                      << " *** ERROR ***  No formation parameter for player " << unum
                      << std::endl;
            break;
        }

        FormationBPN::Param::Net & net = param->net();

        FormationBPN::Param::Net::input_array input;
        FormationBPN::Param::Net::output_array teacher;

        const SampleDataSet::DataCont::const_iterator data_end = M_samples->dataCont().end();
        int loop = 0;
        double ave_err = 0.0;
        double max_err = 0.0;
        bool success = false;
        while ( ++loop <= 5000 )
        {
            ave_err = 0.0;
            max_err = 0.0;
            double data_count = 1.0;
            for ( SampleDataSet::DataCont::const_iterator d = M_samples->dataCont().begin();
                  d != data_end;
                  ++d, data_count += 1.0 )
            {
                double by = d->ball_.y;
                double py = d->players_[unum - 1].y;

                if ( type == Formation::CENTER
                     && by > 0.0 )
                {
                    if ( loop == 2 )
                    {
                        std::cerr << "      unum " << unum
                                  << "  training data Y is reversed"
                                  << std::endl;
                    }
                    by *= -1.0;
                    py *= -1.0;
                }

                input[0] = min_max( 0.0,
                                    d->ball_.x / PITCH_LENGTH + 0.5,
                                    1.0 );
                input[1] = min_max( 0.0,
                                    by / PITCH_WIDTH + 0.5,
                                    1.0 );
                teacher[0] = min_max( 0.0,
                                      d->players_[unum - 1].x / PITCH_LENGTH + 0.5,
                                      1.0 );
                teacher[1] = std::max( 0.0,
                                       std::min( py / PITCH_WIDTH + 0.5, 1.0 ) );

                double err = net.train( input, teacher );
                if ( max_err < err )
                {
                    max_err = err;
                }
                ave_err
                    = ave_err * ( ( data_count - 1.0 ) / data_count )
                    + err / data_count;
            }
#if 0
            if ( loop % 500 == 0 )
            {
                std::cerr << "      Training. player " << unum
                          << "  counter " << loop << std::endl;
            }
#endif
            if ( max_err < 0.003 )
            {
                std::cerr << "  ----> converged. average err=" << ave_err
                          << "  last max err=" << max_err
                          << std::endl;
                success = true;
                //printMessageWithTime( "train. converged. loop=%d", loop );
                break;
            }
        }
        if ( ! success )
        {
            std::cerr << "  *** Failed to converge *** " << std::endl;
            //printMessageWithTime( "train. Failed to converge. player %d", unum );
        }
        std::cerr << "  ----> " << loop
                  << " loop. last average err=" << ave_err
                  << "  last max err=" << max_err
                  << std::endl;
    }
    std::cerr << "FormationBPN::train. Ended!!" << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::shared_ptr< FormationBPN::Param >
FormationBPN::getParam( const int unum )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** invalid unum " << unum
                  << std::endl;
        return std::shared_ptr< FormationBPN::Param >( nullptr );
    }

    if ( M_symmetry_number[unum - 1] > 0 )
    {
        return std::shared_ptr< FormationBPN::Param >( nullptr );
    }

    std::map< int, std::shared_ptr< Param > >::const_iterator it = M_param_map.find( unum );
    if ( it == M_param_map.end() )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** Neteter not found! unum = "
                  << unum << std::endl;
        return std::shared_ptr< Param >( nullptr );
    }

    return it->second;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::shared_ptr< const FormationBPN::Param >
FormationBPN::getParam( const int unum ) const
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** invalid unum " << unum
                  << std::endl;
        return std::shared_ptr< const FormationBPN::Param >( nullptr );
    }

    const int player_number = ( M_symmetry_number[unum - 1] <= 0 // center or original
                                ? unum
                                : M_symmetry_number[unum - 1] );

    std::map< int, std::shared_ptr< Param > >::const_iterator it = M_param_map.find( player_number );
    if ( it == M_param_map.end() )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** Neteter not found! unum = "
                  << unum << std::endl;
        return std::shared_ptr< const FormationBPN::Param >( nullptr );
    }

    return it->second;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
FormationBPN::read( std::istream & is )
{
    if ( ! readHeader( is ) ) return false;
    if ( ! readConf( is ) ) return false;
    if ( ! readSamples( is ) ) return false;

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
bool
FormationBPN::readConf( std::istream & is )
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
FormationBPN::readPlayers( std::istream & is )
{
    int player_read = 0;
    std::string line_buf;

    //---------------------------------------------------
    // read each player's parameter set
    for ( int i = 0; i < 11; ++i )
    {
        if ( ! std::getline( is, line_buf ) )
        {
            break;
        }

        // check id
        int unum = 0;
        int symmetry = 0;
        int n_read = 0;
        if ( std::sscanf( line_buf.c_str(),
                          " player %d %d %n ",
                          &unum, &symmetry,
                          &n_read ) != 2
             || n_read == 0 )
        {
            std::cerr << __FILE__ << ':' << __LINE__<< ':'
                      << " *** ERROR *** failed to read formation at number "
                      << i + 1
                      << " [" << line_buf << "]"
                      << std::endl;
            return false;
        }
        if ( unum != i + 1 )
        {
            std::cerr << __FILE__ << ':' << __LINE__<< ':'
                      << " *** ERROR *** failed to read formation "
                      << " Invalid player number.  Expected " << i + 1
                      << "  but read number = " << unum
                      << std::endl;
            return false;
        }
        if ( symmetry == unum )
        {
            std::cerr << __FILE__ << ':' << __LINE__<< ':'
                      << " *** ERROR *** failed to read formation."
                      << " Invalid symmetry number. at "
                      << i
                      << " mirroing player itself. unum = " << unum
                      << "  symmetry = " << symmetry
                      << std::endl;
            return false;
        }
        if ( 11 < symmetry )
        {
            std::cerr << __FILE__ << ':' << __LINE__ << ':'
                      << " *** ERROR *** failed to read formation."
                      << " Invalid symmetry number. at "
                      << i
                      << "  Symmetry number is out of range. unum = " << unum
                      << "  symmetry = " << symmetry
                      << std::endl;
            return false;
        }

        M_symmetry_number[i] = symmetry;

        // this player is symmetry type
        if ( symmetry > 0 )
        {
            ++player_read;
            continue;
        }

        // read parameters
        std::shared_ptr< FormationBPN::Param > param( new Param() );
        if ( ! param->read( is ) )
        {
            std::cerr << __FILE__ << ':' << __LINE__ << ':'
                      << " *** ERROR *** failed to read formation. at number "
                      << i + 1
                      << " [" << line_buf << "]"
                      << std::endl;
            return false;
        }

        ++player_read;
        M_param_map.insert( std::make_pair( unum, param ) );
    }

    if ( player_read != 11 )
    {
        std::cerr << __FILE__ << ':' << __LINE__ << ':'
                  << " ***ERROR*** Invalid formation format."
                  << " The number of read player is " << player_read
                  << std::endl;
        return false;
    }

    if ( ! std::getline( is, line_buf )
         || line_buf != "End" )
    {
        std::cerr << __FILE__ << ':' << __LINE__ << ':'
                  << " ***ERROR*** Illegal end tag."
                  << std::endl;
        return false;
    }

    return true;
}


/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationBPN::print( std::ostream & os ) const
{
    if ( os ) printHeader( os );
    if ( os ) printConf( os );
    if ( os ) printSamples( os );

    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
FormationBPN::printConf( std::ostream & os ) const
{
    for ( int i = 0; i < 11; ++i )
    {
        const int unum = i + 1;

        os << "player " << unum << ' ' << M_symmetry_number[i] << '\n';

        if ( M_symmetry_number[i] > 0 )
        {
            continue;
        }

        std::map< int, std::shared_ptr< Param > >::const_iterator it = M_param_map.find( unum );
        if ( it == M_param_map.end() )
        {
            std::cerr << __FILE__ << ':' << __LINE__ << ':'
                      << " *** ERROR *** Invalid player Id at number "
                      << i + 1
                      << ".  No formation param!"
                      << std::endl;
        }
        else
        {
            it->second->print( os );
        }
    }

    os << "End" << std::endl;
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
namespace {

Formation::Ptr
create()
{
    Formation::Ptr ptr( new FormationBPN() );
    return ptr;
}

rcss::RegHolder f = Formation::creators().autoReg( &create,
                                                   FormationBPN::NAME );

}

}
