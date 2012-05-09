// -*-c++-*-

/*!
  \file formation_dt.cpp
  \brief Delaunay Triangulation formation data classes Source File.
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

#include "formation_dt.h"

#include <rcsc/geom/segment_2d.h>
#include <rcsc/geom/line_2d.h>
#include <rcsc/math_util.h>

#include <sstream>
#include <cstdio>

namespace rcsc {

using namespace formation;

const std::string FormationDT::NAME( "DelaunayTriangulation" );

namespace {

inline
double
round_coord( const double & val )
{
    return rint( val / SampleData::PRECISION ) * SampleData::PRECISION;
}

}

/*-------------------------------------------------------------------*/
/*!

 */
FormationDT::FormationDT()
    : Formation()
{
    M_version = 2;

    for ( int i = 0; i < 11; ++i )
    {
        M_role_name[i] = "Dummy";
    }
}



/*-------------------------------------------------------------------*/
/*!

 */
void
FormationDT::createDefaultData()
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
    data.players_.push_back( Vector2D( -50.0, 0.0 ) );
    data.players_.push_back( Vector2D( -15.0, 0.0 ) );
    data.players_.push_back( Vector2D( -15.0, -8.0 ) );
    data.players_.push_back( Vector2D( -15.0, 8.0 ) );
    data.players_.push_back( Vector2D( -9.0, 0.0 ) );
    data.players_.push_back( Vector2D( -5.0, -16.0 ) );
    data.players_.push_back( Vector2D( -5.0, 16.0 ) );
    data.players_.push_back( Vector2D( 0.0, -25.0 ) );
    data.players_.push_back( Vector2D( 0.0, 25.0 ) );
    data.players_.push_back( Vector2D( 10.0, -10.0 ) );
    data.players_.push_back( Vector2D( 10.0, 10.0 ) );
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
    data.players_.push_back( Vector2D( -50.0, 0.0 ) );
    data.players_.push_back( Vector2D( -15.0, 0.0 ) );
    data.players_.push_back( Vector2D( -15.0, -8.0 ) );
    data.players_.push_back( Vector2D( -15.0, 8.0 ) );
    data.players_.push_back( Vector2D( -5.0, -16.0 ) );
    data.players_.push_back( Vector2D( -5.0, 16.0 ) );
    data.players_.push_back( Vector2D( -9.0, 0.0 ) );
    data.players_.push_back( Vector2D( 0.0, -25.0 ) );
    data.players_.push_back( Vector2D( 0.0, 25.0 ) );
    data.players_.push_back( Vector2D( 10.0, -10.0 ) );
    data.players_.push_back( Vector2D( 10.0, 10.0 ) );
#endif

    M_samples->addData( *this, data, false );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
FormationDT::setRoleName( const int unum,
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
FormationDT::getRoleName( const int unum ) const
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
FormationDT::createNewRole( const int unum,
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
Vector2D
FormationDT::getPosition( const int unum,
                          const Vector2D & focus_point ) const
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** invalid unum " << unum
                  << std::endl;
        return Vector2D::INVALIDATED;
    }

    const DelaunayTriangulation::Triangle * tri
        = M_triangulation.findTriangleContains( focus_point );

    // linear interpolation
    return interpolate( unum, focus_point, tri );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
FormationDT::getPositions( const Vector2D & focus_point,
                           std::vector< Vector2D > & positions ) const
{
    positions.clear();

    const DelaunayTriangulation::Triangle * tri
        = M_triangulation.findTriangleContains( focus_point );

    for ( int unum = 1; unum <= 11; ++unum )
    {
        positions.push_back( interpolate( unum, focus_point, tri ) );
    }
}


/*-------------------------------------------------------------------*/
/*!

 */
Vector2D
FormationDT::interpolate( const int unum,
                          const Vector2D & focus_point,
                          const DelaunayTriangulation::Triangle * tri ) const
{

    if ( ! tri )
    {
        const DelaunayTriangulation::Vertex * v
            = M_triangulation.findNearestVertex( focus_point );

        if ( ! v )
        {
            std::cerr << __FILE__ << ":" << __LINE__
                      << " *** ERROR *** No vertex!"
                      << std::endl;
            return Vector2D::INVALIDATED;
        }

        try
        {
            //std::cerr << "found nearest vertex id= " << v->id() << std::endl;
            return M_sample_vector.at( v->id() ).getPosition( unum );
        }
        catch ( std::exception & e )
        {
            std::cerr << __FILE__ << ":" << __LINE__
                      << " exception caught! "
                      << e.what() << std::endl;
            return Vector2D::INVALIDATED;
        }
    }

    try
    {
        Vector2D result_0 = M_sample_vector.at( tri->vertex( 0 )->id() ).getPosition( unum );
        Vector2D result_1 = M_sample_vector.at( tri->vertex( 1 )->id() ).getPosition( unum );
        Vector2D result_2 = M_sample_vector.at( tri->vertex( 2 )->id() ).getPosition( unum );

        Line2D line_0( tri->vertex( 0 )->pos(),
                       focus_point );

        Segment2D segment_12( tri->vertex( 1 )->pos(),
                              tri->vertex( 2 )->pos() );

        Vector2D intersection_12 = segment_12.intersection( line_0 );

        if ( ! intersection_12.isValid() )
        {
            if ( focus_point.dist2( tri->vertex( 0 )->pos() ) < 1.0e-5 )
            {
                return result_0;
            }

            std::cerr << __FILE__ << ":" << __LINE__
                      << "***ERROR*** No intersection!\n"
                      << " focus=" << focus_point
                      << " line_intersection=" << intersection_12
                      << "\n v0=" << tri->vertex( 0 )->pos()
                      << " v1=" << tri->vertex( 1 )->pos()
                      << " v2=" << tri->vertex( 2 )->pos()
                      << std::endl;

            return ( result_0 + result_1 + result_2 ) / 3.0;
        }

        // distance from vertex_? to interxection_12
        double dist_1i = tri->vertex( 1 )->pos().dist( intersection_12 );
        double dist_2i = tri->vertex( 2 )->pos().dist( intersection_12 );

        // interpolation result of vertex_1 & vertex_2
        Vector2D result_12
            = result_1
            + ( result_2 - result_1 ) * ( dist_1i / ( dist_1i + dist_2i ) );

        // distance from vertex_0 to ball
        double dist_0b = tri->vertex( 0 )->pos().dist( focus_point );
        // distance from interxectin_12 to ball
        double dist_ib = intersection_12.dist( focus_point );

        // interpolation result of vertex_0 & intersection_12
        Vector2D result_012
            = result_0
            + ( result_12 - result_0 ) * ( dist_0b / ( dist_0b + dist_ib ) );

        return result_012;
    }
    catch ( std::exception & e )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " Exception caught!! "
                  << e.what()
                  << std::endl;
        return  Vector2D::INVALIDATED;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
FormationDT::train()
{
    if ( ! M_samples )
    {
        return;
    }

    Rect2D pitch( Vector2D( -60.0, -45.0 ),
                  Size2D( 120.0, 90.0 ) );
    M_triangulation.init( pitch );
    M_sample_vector.clear();

    const SampleDataSet::DataCont::const_iterator end = M_samples->dataCont().end();
    for ( SampleDataSet::DataCont::const_iterator it = M_samples->dataCont().begin();
          it != end;
          ++it )
    {
        M_triangulation.addVertex( it->ball_ );
        M_sample_vector.push_back( *it );
    }

    M_triangulation.compute();
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationDT::readConf( std::istream & is )
{
    bool result = false;
    if ( version() >= 2 )
    {
        result = readV2( is );
    }
    else
    {
        result = readV1( is );
    }

    if ( version() == 0 )
    {
        M_version = 2;
    }

    ////////////////////////////////////////////////////////
    if ( result )
    {
        train();
    }

    return result;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationDT::readSamples( std::istream & )
{
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationDT::readV1( std::istream & is )
{
    // read role assignment
    if ( ! readRoles( is ) )
    {
        return false;
    }

#if 0
    std::cerr << "Loaded Role Assignment" << std::endl;
    for ( int unum = 1; unum <= 11; ++unum )
    {
        std::cerr << "Role=" << getRoleName( unum )
                  << " Symmetry=" << getSymmetryNumber( unum )
                  << std::endl;
    }
#endif

    //---------------------------------------------------
    // read vertices
    if ( ! readVertices( is ) )
    {
        return false;
    }

    if ( is.eof() )
    {
        std::cerr << "Input stream reaches EOF"
                  << std::endl;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationDT::readRoles( std::istream & is )
{
    std::string line_buf;

    if ( ! std::getline( is, line_buf ) )
    {
        std::cerr << __FILE__ << ':' << __LINE__ << ':'
                  << " *** ERROR *** readRoles(). Failed getline "
                  << std::endl;
        return false;
    }

    const char * msg = line_buf.c_str();

    for ( int unum = 1; unum <= 11; ++unum )
    {
        char role_name[128];
        int symmetry_number;
        int n_read = 0;

        if ( std::sscanf( msg, " %s %d %n ",
                          role_name, &symmetry_number, &n_read ) != 2 )
        {
            std::cerr << __FILE__ << ':' << __LINE__ << ':'
                      << " *** ERROR *** readRoles(). Failed to read player "
                      << unum
                      << std::endl;
            return false;
        }
        msg += n_read;

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

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationDT::readVertices( std::istream & is )
{
    M_samples = SampleDataSet::Ptr( new SampleDataSet() );

    std::string line_buf;

    while ( std::getline( is, line_buf ) )
    {
        if ( line_buf == "End" )
        {
            return true;
        }

        SampleData new_sample;

        const char * msg = line_buf.c_str();

        double read_x, read_y;
        int n_read = 0;

        // read ball pos
        if ( std::sscanf( msg, " %lf %lf %n ",
                          &read_x, &read_y, &n_read ) != 2 )
        {
            std::cerr << __FILE__ << ':' << __LINE__ << ':'
                      << " *** ERROR *** Invalid ball data ["
                      << line_buf << "]"
                      << std::endl;
            return false;
        }
        msg += n_read;

        new_sample.ball_.assign( read_x, read_y );

        for ( int unum = 1; unum <= 11; ++unum )
        {
            if ( std::sscanf( msg, " %lf %lf %n ",
                              &read_x, &read_y, &n_read ) != 2 )
            {
                std::cerr << __FILE__ << ':' << __LINE__ << ':'
                          << " *** ERROR *** Illegal player data. unum = "
                          << unum
                          << " [" << line_buf << "]"
                          << std::endl;
                return false;
            }
            msg += n_read;

            new_sample.players_.push_back( Vector2D( read_x, read_y ) );
        }

        M_samples->addData( *this, new_sample, false );
    }

    return false;
}


/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationDT::readV2( std::istream & is )
{
    if ( ! readRolesV2( is ) )
    {
        return false;
    }

    if ( ! readVerticesV2( is ) )
    {
        return false;
    }

    //
    // read End tag
    //

    std::string line_buf;
    while ( std::getline( is, line_buf ) )
    {
        if ( line_buf.empty()
             || line_buf[0] == '#'
             || ! line_buf.compare( 0, 2, "//" ) )
        {
            continue;
        }

        if ( line_buf != "End" )
        {
            std::cerr << __FILE__ << ':' << __LINE__ << ':'
                      << " *** ERROR *** readV2(). No end tag "
                      << std::endl;
            return false;
        }

        break;
    }

    if ( is.eof() )
    {
        std::cerr << "Input stream reaches EOF"
                  << std::endl;
    }

    return true;
}


/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationDT::readRolesV2( std::istream & is )
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
                      << " *** ERROR *** readRolesV2(). Illegal header ["
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
                      << " *** ERROR *** readRolesV2(). Illegal role data. num="
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
                      << " *** ERROR *** readRolesV2(). Failed getline "
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
bool
FormationDT::readVerticesV2( std::istream & is )
{
    M_samples = SampleDataSet::Ptr( new SampleDataSet() );

    if ( ! M_samples->read( is ) )
    {
        M_samples.reset();
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationDT::printConf( std::ostream & os ) const
{
    return printV2( os );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationDT::printSamples( std::ostream & os ) const
{
    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationDT::printV1( std::ostream & os ) const
{
    // put role assignment
    for ( int unum = 1; unum <= 11; ++unum )
    {
        os << M_role_name[unum - 1] << ' '
           << M_symmetry_number[unum - 1] << ' ';
    }
    os << '\n';

    for ( std::vector< SampleData >::const_iterator it = M_sample_vector.begin();
          it != M_sample_vector.end();
          ++it )
    {
        os << round_coord( it->ball_.x ) << ' '
           << round_coord( it->ball_.y ) << ' ';

        for ( SampleData::PlayerCont::const_iterator p = it->players_.begin();
              p != it->players_.end();
              ++p )
        {
            os << round_coord( p->x ) << ' '
               << round_coord( p->y ) << ' ';
        }
        os << '\n';
    }

    os << "End" << std::endl;
    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationDT::printV2( std::ostream & os ) const
{
    printRolesV2( os );
    printDataV2( os );

    os << "End" << std::endl;
    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationDT::printRolesV2( std::ostream & os ) const
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
std::ostream &
FormationDT::printDataV2( std::ostream & os ) const
{
    M_samples->print( os );
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
namespace {

Formation::Ptr
create()
{
    Formation::Ptr ptr( new FormationDT() );
    return ptr;
}

rcss::RegHolder f = Formation::creators().autoReg( &create,
                                                   FormationDT::NAME );

}

}
