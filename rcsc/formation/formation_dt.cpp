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

namespace rcsc {

const std::string FormationDT::NAME( "DelaunayTriangulation" );

/*-------------------------------------------------------------------*/
FormationDT::FormationDT()
    : Formation()
{

}

/*-------------------------------------------------------------------*/
std::string
FormationDT::methodName() const
{
    return NAME;
}

/*-------------------------------------------------------------------*/
Vector2D
FormationDT::getPosition( const int num,
                          const Vector2D & focus_point ) const
{
    if ( num < 1 || 11 < num )
    {
        std::cerr << "(FormationDT::getPosition) ERROR: invalid number " << num << std::endl;
        return Vector2D::INVALIDATED;
    }

    const DelaunayTriangulation::Triangle * tri = M_triangulation.findTriangleContains( focus_point );

    // linear interpolation
    return interpolate( num, focus_point, tri );
}

/*-------------------------------------------------------------------*/
void
FormationDT::getPositions( const Vector2D & focus_point,
                           std::vector< Vector2D > & positions ) const
{
    positions.clear();

    const DelaunayTriangulation::Triangle * tri = M_triangulation.findTriangleContains( focus_point );

    for ( int num = 1; num <= 11; ++num )
    {
        positions.push_back( interpolate( num, focus_point, tri ) );
    }
}

/*-------------------------------------------------------------------*/
Vector2D
FormationDT::interpolate( const int num,
                          const Vector2D & focus_point,
                          const DelaunayTriangulation::Triangle * tri ) const
{

    if ( ! tri )
    {
        const DelaunayTriangulation::Vertex * v = M_triangulation.findNearestVertex( focus_point );

        if ( ! v )
        {
            std::cerr << "(FormationDT::interpolate) ERROR: No vertex." << std::endl;
            return Vector2D::INVALIDATED;
        }

        try
        {
            //std::cerr << "found nearest vertex id= " << v->id() << std::endl;
            return M_points.at( v->id() ).getPosition( num );
        }
        catch ( std::exception & e )
        {
            std::cerr << "(FormationDT::getPosition) exception " << e.what() << std::endl;
            return Vector2D::INVALIDATED;
        }
    }

    Vector2D result_0;
    Vector2D result_1;
    Vector2D result_2;
    try
    {
        result_0 = M_points.at( tri->vertex( 0 )->id() ).getPosition( num );
        result_1 = M_points.at( tri->vertex( 1 )->id() ).getPosition( num );
        result_2 = M_points.at( tri->vertex( 2 )->id() ).getPosition( num );
    }
    catch ( std::exception & e )
    {
        std::cerr << "(FormationDT::getPosition) Exception " << e.what() << std::endl;
        return  Vector2D::INVALIDATED;
    }

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

        std::cerr << "(FormationDT::getPosition) ERROR: No intersection!\n"
                  << " focus=" << focus_point << " line_intersection=" << intersection_12
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

/*-------------------------------------------------------------------*/
bool
FormationDT::train( const FormationData & data )
{
    Rect2D pitch( Vector2D( -60.0, -45.0 ),
                  Size2D( 120.0, 90.0 ) );
    M_triangulation.init( pitch );
    M_points.clear();

    for ( const FormationData::Data & d : data.dataCont() )
    {
        M_triangulation.addVertex( d.ball_ );
        M_points.push_back( d );
    }

    M_triangulation.compute();
    return true;
}

/*-------------------------------------------------------------------*/
FormationData::Ptr
FormationDT::toData() const
{
    FormationData::Ptr ptr( new FormationData() );

    for ( const FormationData::Data & d : M_points )
    {
        ptr->addData( d );
    }

    return ptr;
}

/*-------------------------------------------------------------------*/
namespace {
const std::string tab = "  ";

/*-------------------------------------------------------------------*/
bool
print_data_element( std::ostream & os,
                    const size_t idx,
                    const FormationData::Data & data )
{
    char buf[128];
    os << tab << tab << "{\n";
    os << tab << tab << tab << "\"index\" : " << idx << ",\n";
    snprintf( buf, sizeof( buf ) - 1,
              "\"ball\" : { \"x\" : % 6.2f, \"y\" : % 6.2f }",
              data.ball_.x, data.ball_.y );
    os << tab << tab << tab << buf;
    // os << tab << tab << tab << "\"ball\" : { "
    //    << "\"x\" : " << data.ball_.x << ", "
    //    << "\"y\" : " << data.ball_.y << " }";

    for ( size_t i = 0; i < data.players_.size(); ++i )
    {
        os << ",\n";
        snprintf( buf, sizeof( buf ) - 1,
                  "  %s\"%zd\" : { \"x\" : % 6.2f, \"y\" : % 6.2f }",
                  ( i < 9 ? " " : "" ), i + 1, data.players_[i].x, data.players_[i].y );
        os << tab << tab << tab << buf;
        // os << '"' << i + 1 << '"' // number
        //    << " : { "
        //    << "\"x\" : " << data.players_[i].x << ", "
        //    << "\"y\" : " << data.players_[i].y
        //    << " }";
    }

    os << '\n' << tab << tab << '}';
    return true;
}

}

/*-------------------------------------------------------------------*/
bool
FormationDT::printData( std::ostream & os ) const
{
    os << tab << "\"data\"" << " : [\n";

    size_t idx = 0;
    for ( const auto & d : M_points )
    {
        if ( idx != 0 ) os << ",\n";

        if ( ! print_data_element( os, idx, d ) )
        {
            return false;
        }
        ++idx;
    }

    os << '\n' << tab << ']';

    return true;
}

}
