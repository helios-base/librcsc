// -*-c++-*-

/*!
  \file formation_knn.cpp
  \brief k-nearest neighbor formation class Source File.
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

#include "formation_knn.h"

#include <rcsc/math_util.h>

#include <algorithm>
#include <cstdio>

namespace rcsc {

using namespace formation;

const std::string FormationKNN::NAME( "k-NN" );

/*-------------------------------------------------------------------*/
/*!

 */
FormationKNN::FormationKNN()
    : Formation()
    , M_k( 3 )
{
    for ( int i = 0; i < 11; ++i )
    {
        M_role_name[i] = "Dummy";
    }

    M_version = 2;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
FormationKNN::createDefaultData()
{
#if 1
    // 4-3-3

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

#elif 0
    // 1-3-4-2

    // 1: goalie
    // 2: sweeper
    // 3: left side back
    // 4: right side back
    // 5: center back
    // 6: left offensive half
    // 7: right offensive half
    // 8: left side half
    // 9: right side half
    // 10: left forward
    // 11: right forward
    createNewRole( 1, "Goalie", Formation::CENTER );
    createNewRole( 2, "Sweeper", Formation::CENTER );
    createNewRole( 3, "SideBack", Formation::SIDE );
    setSymmetryType( 4, 3 );
    createNewRole( 5, "DefensiveHalf", Formation::CENTER );
    createNewRole( 6, "OffensiveHalf", Formation::SIDE );
    setSymmetryType( 7, 6 );
    createNewRole( 8, "SideHalf", Formation::SIDE );
    setSymmetryType( 9, 8 );
    createNewRole( 10, "Forward", Formation::SIDE );
    setSymmetryType( 11, 10 );

    SampleData data;

    data.ball_.assign( 0.0, 0.0 );
    data.players_.emplace_back( -50.0, 0.0 );
    data.players_.emplace_back( -15.0, 0.0 );
    data.players_.emplace_back( -15.0, -8.0 );
    data.players_.emplace_back( -15.0, 8.0 );
    data.players_.emplace_back( -9.0, 0.0 );
    data.players_.emplace_back( -5.0, -16.0 );
    data.players_.emplace_back( -5.0, 16.0 );
    data.players_.emplace_back( 0.0, -25.0 );
    data.players_.emplace_back( 0.0, 25.0 );
    data.players_.emplace_back( 10.0, -10.0 );
    data.players_.emplace_back( 10.0, 10.0 );
#else
    // 3-5-2

    // 1: goalie
    // 2: sweeper
    // 3: left side back
    // 4: right side back
    // 5: left defensive half
    // 6: right defensive half
    // 7: offensive half
    // 8: left side half
    // 9: right side half
    // 10: left forward
    // 11: right forward
    createNewRole( 1, "Goalie", Formation::CENTER );
    createNewRole( 2, "Sweeper", Formation::CENTER );
    createNewRole( 3, "SideBack", Formation::SIDE );
    setSymmetryType( 4, 3 );
    createNewRole( 5, "DefensiveHalf", Formation::SIDE );
    setSymmetryType( 6, 5 );
    createNewRole( 7, "OffensiveHalf", Formation::CENTER );
    createNewRole( 8, "SideHalf", Formation::SIDE );
    setSymmetryType( 9, 8 );
    createNewRole( 10, "Forward", Formation::SIDE );
    setSymmetryType( 11, 10 );

    SampleData data;

    data.ball_.assign( 0.0, 0.0 );
    data.players_.emplace_back( -50.0, 0.0 );
    data.players_.emplace_back( -15.0, 0.0 );
    data.players_.emplace_back( -15.0, -8.0 );
    data.players_.emplace_back( -15.0, 8.0 );
    data.players_.emplace_back( -5.0, -16.0 );
    data.players_.emplace_back( -5.0, 16.0 );
    data.players_.emplace_back( -9.0, 0.0 );
    data.players_.emplace_back( 0.0, -25.0 );
    data.players_.emplace_back( 0.0, 25.0 );
    data.players_.emplace_back( 10.0, -10.0 );
    data.players_.emplace_back( 10.0, 10.0 );
#endif

    M_samples->addData( *this, data, false );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
FormationKNN::setRoleName( const int unum,
                           const std::string & name )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** invalid unum " << unum
                  << std::endl;
        return;
    }

    M_role_name[unum - 1] = name;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::string
FormationKNN::getRoleName( const int unum ) const
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** invalid unum " << unum
                  << std::endl;
        return std::string( "" );
    }

    return M_role_name[unum - 1];
}

/*-------------------------------------------------------------------*/
/*!

 */
void
FormationKNN::createNewRole( const int unum,
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
                  << " ***ERROR*** Unsupported side type "
                  << std::endl;
        break;
    default:
        break;
    }
}


/*-------------------------------------------------------------------*/

/*!
  \class DataCmp
  \brief data compare function object
 */
class DataCmp {
private:
    const Vector2D M_point; //!< center point
public:

    /*!
      \brief construct with center point
      \param point center point
     */
    DataCmp( const Vector2D & point )
        : M_point( point )
      { }

    /*!
      \brief operator function
      \param lhs left hand side object
      \param rhs right hand side object
      \return true if lhs data point is closer than rhs data point
     */
    bool operator()( const SampleData * lhs,
                     const SampleData * rhs )
      {
          return lhs->ball_.dist2( M_point ) < rhs->ball_.dist2( M_point );
      }
};


/*-------------------------------------------------------------------*/
/*!

 */
Vector2D
FormationKNN::getPosition( const int unum,
                           const Vector2D & focus_point ) const
{
    static std::vector< const SampleData * > ptr_vector;

    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** Illegal unum " << unum
                  << std::endl;
        return Vector2D::INVALIDATED;
    }

    if ( M_samples->dataCont().empty() )
    {
        return Vector2D( 0.0, 0.0 );
    }

    ptr_vector.reserve( M_samples->dataCont().size() );

    const SampleDataSet::DataCont::const_iterator d_end = M_samples->dataCont().end();
    for ( SampleDataSet::DataCont::const_iterator d = M_samples->dataCont().begin();
          d != d_end;
          ++d )
    {
        ptr_vector.push_back( &(*d) );
    }

    const size_t size = std::min( ptr_vector.size(), M_k );

    std::partial_sort( ptr_vector.begin(),
                       ptr_vector.begin() + size,
                       ptr_vector.end(),
                       DataCmp( focus_point ) );

    std::vector< double > inv_dist2( size, 0.0 );
    double sum_inv_dist2 = 0.0;

    for ( size_t i = 0; i < size; ++i )
    {
        double d2 = ptr_vector[i]->ball_.dist2( focus_point );
        if ( d2 < 1.0e-10 )
        {
            return ptr_vector[i]->getPosition( unum );
        }
        inv_dist2[i] = 1.0 / d2;
        sum_inv_dist2 += inv_dist2[i];
    }

    Vector2D pos( 0.0, 0.0 );

    for ( size_t i = 0; i < size; ++i )
    {
        pos += ptr_vector[i]->getPosition( unum ) * inv_dist2[i];
    }

    pos /= sum_inv_dist2;

    ptr_vector.clear();

#if 0
    if ( unum == 11 )
    {
        std::cerr << "sum_inv_dist2=" << sum_inv_dist2 << '\n';
        for ( size_t i = 0; i < size; ++i )
        {
            std::cerr << "  ball=" << ptr_vector[i]->ball_
                      << " pos=" << ptr_vector[i]->getPosition( unum )
                      << " inv_dist2=" << inv_dist2[i]
                      << " rate=" << ( inv_dist2[i] / sum_inv_dist2 )
                      << '\n';
        }
        std::cerr << "  pos = " << pos << std::endl;
    }
#endif

    return pos;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
FormationKNN::getPositions( const Vector2D & focus_point,
                            std::vector< Vector2D > & positions ) const
{
    static std::vector< const SampleData * > ptr_vector;

    positions.clear();

    ptr_vector.reserve( M_samples->dataCont().size() );

    const SampleDataSet::DataCont::const_iterator d_end = M_samples->dataCont().end();
    for ( SampleDataSet::DataCont::const_iterator d = M_samples->dataCont().begin();
          d != d_end;
          ++d )
    {
        ptr_vector.push_back( &(*d) );
    }

    const size_t size = std::min( ptr_vector.size(), M_k );

    std::partial_sort( ptr_vector.begin(),
                       ptr_vector.begin() + size,
                       ptr_vector.end(),
                       DataCmp( focus_point ) );

    std::vector< double > inv_dist2( size, 0.0 );
    double sum_inv_dist2 = 0.0;

    for ( size_t i = 0; i < size; ++i )
    {
        double d2 = ptr_vector[i]->ball_.dist2( focus_point );
        if ( d2 < 1.0e-10 )
        {
            for ( int unum = 1; unum <= 11; ++unum )
            {
                positions.push_back( ptr_vector[i]->getPosition( unum ) );
            }
            return;
        }

        inv_dist2[i] = 1.0 / d2;
        sum_inv_dist2 += inv_dist2[i];
    }

    for ( int unum = 1; unum <= 11; ++unum )
    {
        Vector2D pos( 0.0, 0.0 );

        for ( size_t i = 0; i < size; ++i )
        {
            pos += ptr_vector[i]->getPosition( unum ) * inv_dist2[i];
        }

        pos /= sum_inv_dist2;
        positions.push_back( pos );
    }

    ptr_vector.clear();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
FormationKNN::train()
{

}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationKNN::read( std::istream & is )
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
FormationKNN::print( std::ostream & os ) const
{
    if ( os ) printHeader( os );
    if ( os ) printConf( os );
    if ( os ) printSamples( os );

    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationKNN::readConf( std::istream & is )
{
    // read role assignment
    if ( ! readRoles( is ) )
    {
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationKNN::readRoles( std::istream & is )
{
    std::string line_buf;

    //
    // read Begin tag
    //

    while ( std::getline( is, line_buf ) )
    {
        if ( line_buf.empty()
             || line_buf[0] == '#'
             || ! line_buf.compare( 0, 2, "//" ) )
        {
            continue;
        }

        if ( line_buf != "Begin Roles" )
        {
            std::cerr << __FILE__ << ':' << __LINE__ << ':'
                      << " *** ERROR *** readRoles(). Illegal header ["
                      << line_buf << ']'
                      << std::endl;
            return false;
        }

        break;
    }

    //
    // read role data
    //

    for ( int unum = 1; unum <= 11; ++unum )
    {
        while ( std::getline( is, line_buf ) )
        {
            if ( line_buf.empty()
                 || line_buf[0] == '#'
                 || ! line_buf.compare( 0, 2, "//" ) )
            {
                continue;
            }
            break;
        }

        int read_unum = 0;
        char role_name[128];
        int symmetry_number = 0;

        if ( std::sscanf( line_buf.c_str(),
                          " %d %127s %d ",
                          &read_unum, role_name, &symmetry_number ) != 3
             || read_unum != unum )
        {
            std::cerr << __FILE__ << ':' << __LINE__ << ':'
                      << " *** ERROR *** readRoles(). Illegal role data. num="
                      << unum
                      << " [" << line_buf << "]"
                      << std::endl;
            return false;
        }

        //
        // create role or set symmetry.
        //
        const Formation::SideType type = ( symmetry_number == 0
                                           ? Formation::CENTER
                                           : symmetry_number < 0
                                           ? Formation::SIDE
                                           : Formation::SYMMETRY );
        if ( type == Formation::CENTER )
        {
            createNewRole( unum, role_name, type );
        }
        else if ( type == Formation::SIDE )
        {
            createNewRole( unum, role_name, type );
        }
        else
        {
            setSymmetryType( unum, symmetry_number, role_name );
        }
    }

    //
    // read End tag
    //

    while ( std::getline( is, line_buf ) )
    {
        if ( line_buf.empty()
             || line_buf[0] == '#'
             || ! line_buf.compare( 0, 2, "//" ) )
        {
            continue;
        }

        if ( line_buf != "End Roles" )
        {
            std::cerr << __FILE__ << ':' << __LINE__ << ':'
                      << " *** ERROR *** readRoles(). Failed getline "
                      << std::endl;
            return false;
        }

        break;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationKNN::printConf( std::ostream & os ) const
{
    printRoles( os );
    M_samples->print( os );

    os << "End" << std::endl;
    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationKNN::printRoles( std::ostream & os ) const
{
    os << "Begin Roles\n";

    for ( int unum = 1; unum <= 11; ++unum )
    {
        os << unum << ' '
           << M_role_name[unum - 1] << ' '
           << M_symmetry_number[unum - 1] << '\n';
    }

    os << "End Roles\n";

    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
namespace {

Formation::Ptr
create()
{
    Formation::Ptr ptr( new FormationKNN() );
    return ptr;
}

rcss::RegHolder f = Formation::creators().autoReg( &create,
                                                   FormationKNN::NAME );

}

}
