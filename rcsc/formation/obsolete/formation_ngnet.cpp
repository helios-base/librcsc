// -*-c++-*-

/*!
  \file formation_ngnet.cpp
  \brief NGNet formation data classes Source File.
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

#include "formation_ngnet.h"

#include <rcsc/math_util.h>

#include <sstream>
#include <algorithm>
#include <cstdio>

namespace rcsc {

using namespace formation;

const std::string FormationNGNet::NAME( "NGNet" );

const double FormationNGNet::Param::PITCH_LENGTH = 105.0 + 10.0;
const double FormationNGNet::Param::PITCH_WIDTH = 68.0 + 10.0;


/*-------------------------------------------------------------------*/
/*!

 */
FormationNGNet::Param::Param()
    : M_net()
{

}

/*-------------------------------------------------------------------*/
/*!

 */
Vector2D
FormationNGNet::Param::getPosition( const Vector2D & ball_pos,
                                    const Formation::SideType ) const
{
    NGNet::input_vector input;

    input[0] = ball_pos.x;
    input[1] = ball_pos.y;

    NGNet::output_vector output;

    M_net.propagate( input, output );

    return Vector2D( bound( -PITCH_LENGTH*0.5, output[0], PITCH_LENGTH*0.5 ),
                     bound( -PITCH_WIDTH*0.5, output[1], PITCH_WIDTH*0.5 ) );
}

/*-------------------------------------------------------------------*/
/*!
  Format:  Role <RoleNameStr>
*/
bool
FormationNGNet::Param::readRoleName( std::istream & is )
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
FormationNGNet::Param::readParam( std::istream & is )
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
FormationNGNet::Param::read( std::istream & is )
{
    // read role name
    if ( ! readRoleName( is ) )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " Failed to read role name" << std::endl;
        return false;
    }

    if ( ! readParam( is ) )
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
FormationNGNet::Param::printRoleName( std::ostream & os ) const
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
FormationNGNet::Param::printParam( std::ostream & os ) const
{
    M_net.print( os ) << '\n';
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
FormationNGNet::Param::print( std::ostream & os ) const
{
    printRoleName( os );
    printParam( os );

    return os << std::flush;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

 */
FormationNGNet::FormationNGNet()
    : Formation()
{

}

/*-------------------------------------------------------------------*/
/*!

 */
void
FormationNGNet::createDefaultData()
{
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
FormationNGNet::setRoleName( const int unum,
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
FormationNGNet::getRoleName( const int unum ) const
{
    const std::shared_ptr< const Param > p = param( unum );
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
FormationNGNet::createNewRole( const int unum,
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
        // symmetry
    }

    // erase old parameter, if exist
    std::map< int, std::shared_ptr< Param > >::iterator it = M_param_map.find( unum );
    if ( it != M_param_map.end() )
    {
        M_param_map.erase( it );
    }

    std::shared_ptr< FormationNGNet::Param > param( new Param() );
    param->setRoleName( role_name );

    M_param_map.insert( std::make_pair( unum, param ) );
}

/*-------------------------------------------------------------------*/
/*!

 */
Vector2D
FormationNGNet::getPosition( const int unum,
                             const Vector2D & ball_pos ) const
{
    const std::shared_ptr< const Param > ptr = param( unum );
    if ( ! ptr )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " *** ERROR *** FormationNGNet::Param not found. unum = "
                  << unum
                  << std::endl;
        return Vector2D( 0.0, 0.0 );
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
FormationNGNet::getPositions( const Vector2D & focus_point,
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
FormationNGNet::train()
{
    if ( ! M_samples
         || M_samples->dataCont().empty() )
    {
        return;
    }

    std::cerr << "FormationNGNet::train. Started!!" << std::endl;

    for ( int unum = 1; unum <= 11; ++unum )
    {
        int number = unum;

        std::shared_ptr< Param > param = getParam( number );
        if ( ! param )
        {
            std::cerr << __FILE__ << ": " << __LINE__
                      << " *** ERROR ***  No formation parameter for player " << unum
                      << std::endl;
            break;
        }

        NGNet & net = param->getNet();
        std::cerr << "---------- FormationNGNet::train. " << unum
                  << " current unit size = "
                  << net.units().size()
                  << std::endl;

        if ( net.units().size() < M_samples->dataCont().size() )
        {
            SampleDataSet::DataCont::const_iterator d = M_samples->dataCont().begin();
            int count = net.units().size();
            std::cerr << "FormationNGNet::train. need to add new center "
                      << M_samples->dataCont().size() - net.units().size() << std::endl;
            while ( --count >= 0
                    && d != M_samples->dataCont().end() )
            {
                std::cerr << "FormationNGNet::train. skip known data...." << std::endl;
                ++d;
            }

            while ( d != M_samples->dataCont().end() )
            {
                std::cerr << "FormationNGNet::train. added new center "
                          << d->ball_
                          << std::endl;
                NGNet::input_vector center;
                //center[0] = bound( 0.0,
                //                   d->ball_.x / PITCH_LENGTH + 0.5,
                //                   1.0 );
                center[0] = d->ball_.x;
                //center[1] = bound( 0.0,
                //                   d->ball_.y / PITCH_WIDTH + 0.5,
                //                   1.0 );
                center[1] = d->ball_.y;
                net.addCenter( center );

                ++d;
            }
        }

        net.printUnits( std::cerr );

        NGNet::input_vector input;
        NGNet::output_vector teacher;

        const SampleDataSet::DataCont::const_iterator d_end = M_samples->dataCont().end();
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
                  d != d_end;
                  ++d, data_count += 1.0 )
            {
                /*
                  input[0] = bound( 0.0,
                  snap->ball_.x / PITCH_LENGTH + 0.5,
                  1.0 );
                  input[1] = bound( 0.0,
                  snap->ball_.y / PITCH_WIDTH + 0.5,
                  1.0 );
                  teacher[0] = bound( 0.0,
                  snap->players_[unum - 1].x / PITCH_LENGTH + 0.5,
                  1.0 );
                  teacher[1] = bound( 0.0,
                  snap->players_[unum - 1].y / PITCH_WIDTH + 0.5,
                  1.0 );
                */
                input[0] = d->ball_.x;
                input[1] = d->ball_.y;
                teacher[0] = d->players_[unum - 1].x;
                teacher[1] = d->players_[unum - 1].y;

                if ( loop == 2 )
                {
                    std::cerr << "  ----> " << unum
                              << "  ball = " << input[0] << ", " << input[1]
                              << "  teacher = " << teacher[0] << ", " << teacher[1]
                              << std::endl;
                }

                double err = net.train( input, teacher );
                if ( max_err < err )
                {
                    max_err = err;
                }
                ave_err
                    = ave_err * ( ( data_count - 1.0 ) / data_count )
                    + err / data_count;
            }

            if ( max_err < 0.001 )
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
        net.printUnits( std::cerr );
    }
    std::cerr << "FormationNGNet::train. Ended!!" << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationNGNet::read( std::istream & is )
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
std::ostream &
FormationNGNet::print( std::ostream & os ) const
{
    if ( os ) printHeader( os );
    if ( os ) printConf( os );
    if ( os ) printSamples( os );

    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::shared_ptr< FormationNGNet::Param >
FormationNGNet::getParam( const int unum )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** invalid unum " << unum
                  << std::endl;
        return std::shared_ptr< FormationNGNet::Param >( nullptr );
    }

    std::map< int, std::shared_ptr< Param > >::const_iterator it = M_param_map.find( unum );
    if ( it == M_param_map.end() )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** Parameter not found! unum = "
                  << unum << std::endl;
        return std::shared_ptr< Param >( nullptr );
    }

    return it->second;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::shared_ptr< const FormationNGNet::Param >
FormationNGNet::param( const int unum ) const
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** invalid unum " << unum
                  << std::endl;
        return std::shared_ptr< const Param >( nullptr );
    }

    std::map< int, std::shared_ptr< Param > >::const_iterator it = M_param_map.find( unum );
    if ( it == M_param_map.end() )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** Parameter not found! unum = "
                  << unum << std::endl;
        return std::shared_ptr< const Param >( nullptr );
    }

    return it->second;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationNGNet::readConf( std::istream & is )
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
FormationNGNet::readPlayers( std::istream & is )
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
            std::cerr << __FILE__ << ":" << __LINE__
                      << " *** ERROR *** failed to read formation at number "
                      << i + 1
                      << " [" << line_buf << "]"
                      << std::endl;
            return false;
        }
        if ( unum != i + 1 )
        {
            std::cerr << __FILE__ << ":" << __LINE__
                      << " *** ERROR *** failed to read formation "
                      << " Invalid player number.  Expected " << i + 1
                      << "  but read number = " << unum
                      << std::endl;
            return false;
        }
        if ( symmetry == unum )
        {
            std::cerr << __FILE__ << ":" << __LINE__
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
            std::cerr << __FILE__ << ":" << __LINE__
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
        /*
          if ( symmetry > 0 )
          {
          ++player_read;
          continue;
          }
        */

        // read parameters
        std::shared_ptr< Param > param( new Param() );
        if ( ! param->read( is ) )
        {
            std::cerr << __FILE__ << ":" << __LINE__
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
        std::cerr << __FILE__ << ':' << __LINE__
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
FormationNGNet::printConf( std::ostream & os ) const
{
    for ( int i = 0; i < 11; ++i )
    {
        const int unum = i + 1;
        os << "player " << unum << " " << M_symmetry_number[i] << '\n';
        /*
          if ( M_symmetry_number[i] > 0 )
          {
           continue;
          }
        */

        std::map< int, std::shared_ptr< Param > >::const_iterator it = M_param_map.find( unum );
        if ( it == M_param_map.end() )
        {
            std::cerr << __FILE__ << ":" << __LINE__
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
    Formation::Ptr ptr( new FormationNGNet() );
    return ptr;
}

rcss::RegHolder f = Formation::creators().autoReg( &create,
                                                   FormationNGNet::NAME );

}

}
