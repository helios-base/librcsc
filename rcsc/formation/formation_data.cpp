// -*-c++-*-

/*!
  \file sample_data.cpp
  \brief formation sample data class Source File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3, or (at your option)
 any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "formation_data.h"

#include <rcsc/geom/segment_2d.h>

#include <iterator>
#include <algorithm>
#include <limits>
#include <fstream>
#include <sstream>
#include <cstring>

namespace rcsc {

const double FormationData::PRECISION = 0.01;
const size_t FormationData::MAX_DATA_SIZE = 128;
const double FormationData::NEAR_DIST_THR = 0.5;

/*-------------------------------------------------------------------*/
double
FormationData::round_xy( const double xy )
{
    return rint( xy / PRECISION ) * PRECISION;
}

/*-------------------------------------------------------------------*/
Vector2D
FormationData::rounded_vector( const double x,
                               const double y )
{
    return Vector2D( round_xy( x ), round_xy( y ) );
}

/*-------------------------------------------------------------------*/
FormationData::FormationData()
{

}

/*-------------------------------------------------------------------*/
void
FormationData::clear()
{
    M_data_cont.clear();
}

/*-------------------------------------------------------------------*/
const
FormationData::Data *
FormationData::data( const size_t idx ) const
{
    if ( M_data_cont.empty()
         || M_data_cont.size() < idx )
    {
        return nullptr;
    }

    FormationData::DataCont::const_iterator it = M_data_cont.begin();
    std::advance( it, idx );

    return &(*it);
}

/*-------------------------------------------------------------------*/
int
FormationData::nearestDataIndex( const Vector2D & pos,
                                 const double thr ) const
{
    const double dist_thr2 = std::pow( thr, 2 );

    const FormationData::Data * result = nullptr;
    double min_dist2 = std::numeric_limits< double >::max();

    for ( const FormationData::Data & d : M_data_cont )
    {
        double d2 = d.ball_.dist2( pos );

        if ( d2 < dist_thr2
             && d2 < min_dist2 )
        {
            min_dist2 = d2;
            result = &d;
        }
    }

    return ( result
             ? result->index_
             : -1 );
}

/*-------------------------------------------------------------------*/
bool
FormationData::existTooNearData( const FormationData::Data & data ) const
{
    const double dist_thr2 = NEAR_DIST_THR * NEAR_DIST_THR;

    for ( const FormationData::Data & d : M_data_cont )
    {
        if ( d.ball_.dist2( data.ball_ ) < dist_thr2 )
        {
            return true;
        }
    }

    return false;
}

/*-------------------------------------------------------------------*/
void
FormationData::updateDataIndex()
{
    int index = 0;
    for ( FormationData::Data & data : M_data_cont )
    {
        data.index_ = index;
        ++index;
    }
}

/*-------------------------------------------------------------------*/
std::string
FormationData::addData( const FormationData::Data & data )
{
    if ( M_data_cont.size() >= MAX_DATA_SIZE )
    {
        return std::string( "Too many data" );
    }

    if ( existTooNearData( data ) )
    {
        return std::string( "Too near data" );
    }

    //
    // add data
    //
    M_data_cont.push_back( data );

    // std::cerr << "Added data. current data size = " << M_data_cont.size()
    //           << std::endl;


    updateDataIndex();

    return std::string();
}

/*-------------------------------------------------------------------*/
std::string
FormationData::insertData( const size_t idx,
                           const FormationData::Data & data )
{
    if ( M_data_cont.size() >= MAX_DATA_SIZE )
    {
        return std::string( "Too many data" );
    }

    if ( M_data_cont.size() < idx )
    {
        return std::string( "Over insert range" );
    }

    if ( existTooNearData( data ) )
    {
        return std::string( "Too near data" );
    }

    //
    // insert data
    //
    DataCont::iterator it = M_data_cont.begin();
    std::advance( it, idx );

    M_data_cont.insert( it, data );

    std::cerr << "Inserted data at index=" << std::distance( M_data_cont.begin(), it ) + 1
              << ". current data size = " << M_data_cont.size()
              << std::endl;

    updateDataIndex();

    return std::string();
}

/*-------------------------------------------------------------------*/
std::string
FormationData::replaceData( const size_t idx,
                            const FormationData::Data & data )
{
    if ( M_data_cont.size() < idx )
    {
        return std::string( "Invalid index" );
    }

    DataCont::iterator replaced = M_data_cont.begin();
    std::advance( replaced, idx );

    //
    // check near data
    //
    {
        const double dist_thr2 = NEAR_DIST_THR * NEAR_DIST_THR;
        for ( DataCont::iterator it = M_data_cont.begin(), end = M_data_cont.end();
              it != end;
              ++it )
        {
            if ( it != replaced
                 && it->ball_.dist2( data.ball_ ) < dist_thr2 )
            {
                return std::string( "Too near data" );
            }
        }
    }

    FormationData::Data original_data = *replaced;
    *replaced = data;

    std::cerr << "Replaced data at index=" << idx << std::endl;

    //
    // update index value
    //
    updateDataIndex();

    return std::string();
}

/*-------------------------------------------------------------------*/
std::string
FormationData::removeData( const size_t idx )
{
    if ( M_data_cont.size() < idx )
    {
        return std::string( "Invalid index" );
    }

    DataCont::iterator it = M_data_cont.begin();
    std::advance( it, idx );

    //
    // remove the data
    //
    M_data_cont.erase( it );

    //
    updateDataIndex();

    return std::string();
}

/*-------------------------------------------------------------------*/
std::string
FormationData::changeDataIndex( const size_t old_idx,
                                const size_t new_idx )
{
    if ( old_idx == new_idx
         || M_data_cont.size() < old_idx
         || M_data_cont.size() < new_idx )
    {
        return std::string( "Invalid index" );
    }

    DataCont::iterator oit = M_data_cont.begin();
    std::advance( oit, old_idx );

    DataCont::iterator nit = M_data_cont.begin();
    std::advance( nit, new_idx );

    DataCont tmp_list;
    tmp_list.splice( tmp_list.end(), M_data_cont, oit );
    M_data_cont.splice( nit, tmp_list );

    updateDataIndex();

    return std::string();
}

/*-------------------------------------------------------------------*/
std::ostream &
FormationData::print( std::ostream & os ) const
{
    os << "{\n"
       << "  \"data\" : [\n";

    size_t idx = 0;
    for ( const Data & d : M_data_cont )
    {
        if ( idx != 0 ) os << ",\n";

        os << "    {\n";
        os << "      \"index\" : " << idx << ",\n";
        os << "      \"ball\" : { "
           << "\"x\" : " << d.ball_.x << ", "
           << "\"y\" : " << d.ball_.y << " }";

        for ( size_t i = 0; i < d.players_.size(); ++i )
        {
            os << ",\n";
            os << "      \"" << i + 1 << '"' // number
               << " : { "
               << "\"x\" : " << d.players_[i].x << ", "
               << "\"y\" : " << d.players_[i].y
               << " }";
        }

        os << "\n    }";
        ++idx;
    }

    os << "\n  ]\n"
       << "}\n";
    return os;
}

}

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace rcsc {

/*-------------------------------------------------------------------*/
bool
FormationData::read( std::istream & is )
{
    using namespace boost::property_tree;

    ptree doc;
    try
    {
        boost::property_tree::read_json( is, doc );
    }
    catch ( std::exception & e )
    {
        std::cerr << "(FormationData::read) ERROR\n" << e.what() << std::endl;
        return false;
    }

    boost::optional< ptree & > data_array = doc.get_child_optional( "data" );
    if ( ! data_array )
    {
        std::cerr << "(FormationData::read) No data array" << std::endl;
        return false;
    }

    for ( const ptree::value_type & child : *data_array )
    {
        const ptree & elem = child.second;

        Data data;

        try
        {
            data.ball_.assign( elem.get< double >( "ball.x" ),
                               elem.get< double >( "ball.y" ) );

            for ( int i = 0; i < 11; ++i )
            {
                const std::string unum = std::to_string( i + 1 );
                data.players_.emplace_back( elem.get< double >( unum + ".x" ),
                                            elem.get< double >( unum + ".y" ) );
            }
        }
        catch ( std::exception & e )
        {
            std::cerr << "(FormationData::read) " << e.what() << std::endl;
            return false;
        }

        const std::string err = addData( data );
        if ( ! err.empty() )
        {
            std::cerr << "(FormationData::read) ERROR: " << err << std::endl;
            return false;
        }
    }

    return true;
}

}
