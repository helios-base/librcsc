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

#include <boost/algorithm/string.hpp>

#include <vector>
#include <sstream>
#include <cstdio>

namespace rcsc {

const std::string FormationDT::NAME( "DelaunayTriangulation" );

namespace {

/*-------------------------------------------------------------------*/
/*!

 */
inline
double
round_coord( const double & val )
{
    return rint( val / FormationData::PRECISION ) * FormationData::PRECISION;
}

/*-------------------------------------------------------------------*/
/*!

 */
inline
bool
is_comment_line( const std::string & line )
{
    return ( line.empty()
             || line[0] == '#'
             || ! line.compare( 0, 2, "//" ) );
}

}

/*-------------------------------------------------------------------*/
/*!

 */
FormationDT::FormationDT()
    : Formation()
{
    M_version = 3;

    for ( int i = 0; i < 11; ++i )
    {
        M_role_names[i] = "Dummy";
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

    FormationData::Data data;

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

    M_data->addData( data );
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

    M_role_names[unum - 1] = name;
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

    return M_role_names[unum - 1];
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
    if ( ! M_data )
    {
        return;
    }

    Rect2D pitch( Vector2D( -60.0, -45.0 ),
                  Size2D( 120.0, 90.0 ) );
    M_triangulation.init( pitch );
    M_sample_vector.clear();

    for ( const FormationData::Data & data : M_data->dataCont() )
    {
        M_triangulation.addVertex( data.ball_ );
        M_sample_vector.push_back( data );
    }

    M_triangulation.compute();
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationDT::generateModel()
{
    train();
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
FormationDT::createRoleOrSetSymmetry( const int unum,
                                      const std::string & role_name,
                                      const int symmetry_number )
{
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

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationDT::read( std::istream & is )
{
    return readCSV( is );
    //return readV2( is );
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationDT::readOld( std::istream & is )
{
    return readV3( is );
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationDT::readCSV( std::istream & is )
{
    std::vector< int > role_numbers;
    std::vector< std::string > role_names;
    std::vector< std::string > role_types;
    std::vector< int > symmetry_numbers;
    std::vector< int > marker_flags;
    std::vector< int > setplay_marker_flags;

    if ( ! readMethodName( is ) ) return false;

    std::string line_buf;
    std::vector< std::string > values;

    while ( std::getline( is, line_buf ) )
    {
        if ( line_buf.empty()
             || line_buf.front() == '#' )
        {
            continue;
        }

        boost::split( values, line_buf, boost::is_any_of( " ," ) );

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
                    role_numbers.push_back( std::stoi( values[i] ) );
                }
                catch ( std::exception & e )
                {
                    std::cerr << __FILE__ << ' ' << e.what() << '\n'
                              <<" (readCSV) ERROR Illegal # of role number." << std::endl;
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
        }
        else if ( values.front() == "RoleType" )
        {
            role_types.assign( values.begin() + 1, values.end() );
            if ( role_types.size() != 11 )
            {
                std::cerr << __FILE__ << " (readCSV) ERROR Illegal # of role types." << std::endl;
                return false;
            }
        }
        else if ( values.front() == "SymmetryNumber" )
        {
            if ( values.size() != 12 )
            {
                std::cerr << __FILE__ << " (readCSV) ERROR Illegal # of symmetry numbers." << std::endl;
                return false;
            }

            for ( int i = 1; i <= 11; ++i )
            {
                try
                {
                    symmetry_numbers.push_back( std::stoi( values[i] ) );
                }
                catch ( std::exception & e )
                {
                    std::cerr << __FILE__ << ' ' << e.what() << '\n'
                              <<" (readCSV) ERROR Illegal symmetry number." << std::endl;
                    return false;
                }
            }

        }
        else if ( values.front() == "Marker" )
        {
            if ( values.size() != 12 )
            {
                std::cerr << __FILE__ << " (readCSV) ERROR Illegal # of marker flags." << std::endl;
                return false;
            }

            for ( int i = 1; i <= 11; ++i )
            {
                try
                {
                    marker_flags.push_back( std::stoi( values[i] ) );
                }
                catch ( std::exception & e )
                {
                    std::cerr << __FILE__ << ' ' << e.what() << '\n'
                              <<" (readCSV) ERROR Illegal marker flag." << std::endl;
                    return false;
                }
            }
        }
        else if ( values.front() == "SetplayMarker" )
        {
            if ( values.size() != 12 )
            {
                std::cerr << __FILE__ << " (readCSV) ERROR Illegal # of setplay marker flags." << std::endl;
                return false;
            }

            for ( int i = 1; i <= 11; ++i )
            {
                try
                {
                    setplay_marker_flags.push_back( std::stoi( values[i] ) );
                }
                catch ( std::exception & e )
                {
                    std::cerr << __FILE__ << ' ' << e.what() << '\n'
                              <<" (readCSV) ERROR Illegal setplay marker." << std::endl;
                    return false;
                }
            }
        }
        else if ( values.front() == "SampleData" )
        {
            readSamplesCSV( is );
        }
        else
        {
            std::cerr << __FILE__ << " (readCSV) Unsupported data line. [" << values.front() << ']' << std::endl;
            return false;
        }

        values.clear();

    }

    if ( role_numbers.size() != 11
         || role_names.size() != 11
         || role_types.size() != 11
         || symmetry_numbers.size() != 11
         || marker_flags.size() != 11
         || setplay_marker_flags.size() != 11 )
    {
        std::cerr << __FILE__ << " (readCSV) ERROR Illegal # of data." << std::endl;
        return false;
    }

    for ( int i = 0; i < 11; ++i )
    {
        createRoleOrSetSymmetry( role_numbers[i],
                                 role_names[i],
                                 symmetry_numbers[i] );
        setRoleType( role_numbers[i], role_types[i] );
        setMarker( role_numbers[i],
                   marker_flags[i] == 0 ? "x" : "marker",
                   setplay_marker_flags[i] == 0 ? "x" : "setplay_marker" );

    }


    if ( ! checkSymmetryNumber() )
    {
        std::cerr << __FILE__ << " *** ERROR *** Illegal symmetry data."
                  << std::endl;
        return false;
    }

    if ( ! generateModel() ) return false;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationDT::print( std::ostream & os ) const
{
    return printCSV( os );
}


/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationDT::printOld( std::ostream & os ) const
{
    return printV3( os );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationDT::printV3( std::ostream & os ) const
{
    if ( os ) printHeader( os );
    if ( os ) printRolesV3( os );
    if ( os ) printSamplesOld( os );
    if ( os ) printEnd( os );

    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationDT::printCSV( std::ostream & os ) const
{
    printMethodName( os );
    printRoleNumbers( os );
    printRoleNames( os );
    printRoleTypes( os );
    printSymmetryNumbers( os );
    printMarkerFlags( os );
    printSetplayMarkerFlags( os );
    printSamplesCSV( os );

    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationDT::printRoleNumbers( std::ostream & os ) const
{
    os << "RoleNumber";
    for ( int i = 1; i <= 11; ++i )
    {
        os << ',' << i;
    }
    os << '\n';

    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationDT::printRoleNames( std::ostream & os ) const
{
    os << "RoleName";
    for ( int unum = 1; unum <= 11; ++unum )
    {
        os << ',' << M_role_names[unum - 1];
    }
    os << '\n';
    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationDT::printRoleTypes( std::ostream & os ) const
{
    os << "RoleType";
    for ( int unum = 1; unum <= 11; ++unum )
    {
        os << ( M_role_type[unum - 1].isGoalie() ? ",G"
                : M_role_type[unum - 1].isDefender() ? ",DF"
                : M_role_type[unum - 1].isMidFielder() ? ",MF"
                : M_role_type[unum - 1].isForward() ? ",FW"
                : ",U" );
    }
    os << '\n';
    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationDT::printMarkerFlags( std::ostream & os ) const
{
    os << "Marker";
    for ( int unum = 1; unum <= 11; ++unum )
    {
        os << ',' << M_marker[unum-1];
    }
    os << '\n';
    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationDT::printSetplayMarkerFlags( std::ostream & os ) const
{
    os << "SetplayMarker";
    for ( int unum = 1; unum <= 11; ++unum )
    {
        os << ',' << M_setplay_marker[unum-1];
    }
    os << '\n';
    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationDT::printSymmetryNumbers( std::ostream & os ) const
{
    os << "SymmetryNumber";
    for ( int unum = 1; unum <= 11; ++unum )
    {
        os << ',' << M_symmetry_number[unum-1];
    }
    os << '\n';
    return os;
}


/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationDT::readV3( std::istream & is )
{
    if ( ! readHeader( is ) ) return false;
    if ( ! readConf( is ) ) return false;
    if ( ! readSamplesOld( is ) ) return false;
    if ( ! readEnd( is ) ) return false;

    if ( ! checkSymmetryNumber() )
    {
        std::cerr << __FILE__ << " *** ERROR *** Illegal symmetry data."
                  << std::endl;
        return false;
    }

    if ( ! generateModel() ) return false;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationDT::readConf( std::istream & is )
{
    bool result = false;
    if ( version() >= 3 )
    {
        result = readRolesV3( is );
    }
    else
    {
        std::cerr << __FILE__ << " (readConf) unsupported version "
                  << version() << std::endl;
        return false;
    }

    //
    // overwrite format version
    //
    if ( version() <= 2 )
    {
        M_version = 3;
    }

    return result;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationDT::readBeginRolesTag( std::istream & is )
{
    std::string line_buf;
    while ( std::getline( is, line_buf ) )
    {
        if ( is_comment_line( line_buf ) )
        {
            continue;
        }

        if ( line_buf != "Begin Roles" )
        {
            std::cerr << __FILE__ << ':' << __LINE__ << ':'
                      << " *** ERROR *** (readBeginRolesTag) unexpected string ["
                      << line_buf << ']' << std::endl;
            return false;
        }

        return true;
    }

    std::cerr << __FILE__ << ':' << __LINE__ << ':'
              << " *** ERROR *** (readEndRolesTag) 'End Roles' not found"
              << std::endl;

    return false;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationDT::readEndRolesTag( std::istream & is )
{
    std::string line_buf;
    while ( std::getline( is, line_buf ) )
    {
        if ( is_comment_line( line_buf ) )
        {
            continue;
        }

        if ( line_buf != "End Roles" )
        {
            std::cerr << __FILE__ << ':' << __LINE__ << ':'
                      << " *** ERROR *** (readEndRolesTag) unexpected string ["
                      << line_buf << ']' << std::endl;
            return false;
        }

        // found
        return true;
    }

    std::cerr << __FILE__ << ':' << __LINE__ << ':'
              << " *** ERROR *** (readEndRolesTag) 'End Roles' not found"
              << std::endl;

    return false;
}


/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationDT::readEnd( std::istream & is )
{
    std::string line_buf;
    while ( std::getline( is, line_buf ) )
    {
        if ( is_comment_line( line_buf ) )
        {
            continue;
        }

        if ( line_buf != "End" )
        {
            std::cerr << __FILE__ << ':' << __LINE__ << ':'
                      << " *** ERROR *** (readEnd) unexpected string ["
                      << line_buf << ']' << std::endl;
            return false;
        }

        // found
        return true;
    }

    std::cerr << __FILE__ << ':' << __LINE__ << ':'
              << " *** ERROR *** (readEnd) 'End' not found"
              << std::endl;
    if ( is.eof() )
    {
        std::cerr << "Input stream reaches EOF"
                  << std::endl;
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationDT::readRolesV3( std::istream & is )
{
    //
    // read Begin tag
    //

    if ( ! readBeginRolesTag( is ) )
    {
        return false;
    }


    //
    // read role data
    //

    for ( int unum = 1; unum <= 11; ++unum )
    {
        std::string line_buf;
        while ( std::getline( is, line_buf ) )
        {
            if ( is_comment_line( line_buf ) )
            {
                continue;
            }
            break;
        }

        int read_unum = 0;
        char role_name[128];
        int symmetry_number = 0;
        char role_type[4];
        char marker[32];
        char smarker[32];

        if ( std::sscanf( line_buf.c_str(),
                          " %d %3s %127s %d %31s %31s ",
                          &read_unum, role_type, role_name, &symmetry_number, marker, smarker ) != 6
             || read_unum != unum )
        {
            std::cerr << __FILE__ << ':' << __LINE__ << ':'
                      << " *** ERROR *** (readRolesV3). Illegal role data "
                      << " [" << line_buf << "]" << std::endl;
            return false;
        }

        createRoleOrSetSymmetry( unum, role_name, symmetry_number );
        setRoleType( unum, role_type );
        setMarker( unum, marker, smarker );
    }

    //
    // read End tag
    //
    if ( ! readEndRolesTag( is ) )
    {
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationDT::printRolesV3( std::ostream & os ) const
{
    os << "Begin Roles\n";

    os << "# unum  type  name  reference_unum  marker setplay_marker\n";

    for ( int unum = 1; unum <= 11; ++unum )
    {
        os << unum << ' '
           << ( M_role_type[unum - 1].isGoalie() ? "G  "
                : M_role_type[unum - 1].isDefender() ? "DF "
                : M_role_type[unum - 1].isMidFielder() ? "MF "
                : M_role_type[unum - 1].isForward() ? "FW "
                : "U  " )
           << M_role_names[unum - 1] << ' '
           << M_symmetry_number[unum - 1] << ' '
           << ( M_marker[unum-1] ? "marker" : "x" ) << ' '
           << ( M_setplay_marker[unum-1] ? "setplay_marker" : "x" )
           << '\n';
    }

    os << "End Roles\n";

    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationDT::printEnd( std::ostream & os ) const
{
    //os << "End" << std::endl;
    return os;
}



/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationDT::readV2( std::istream & is )
{
    if ( ! readHeader( is ) )
    {
        return false;
    }

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
    M_data = FormationData::Ptr( new FormationData() );

    if ( ! M_data->readOld( is ) )
    {
        M_data.reset();
        return false;
    }

    return true;
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
