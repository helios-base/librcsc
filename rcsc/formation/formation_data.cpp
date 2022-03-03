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

namespace {

inline
double
round_coord( const double & val )
{
    return rint( val / FormationData::PRECISION ) * FormationData::PRECISION;
}

inline
Vector2D
round_coordinates( const double & x,
                   const double & y )
{
    return Vector2D( rint( x / FormationData::PRECISION ) * FormationData::PRECISION,
                     rint( y / FormationData::PRECISION ) * FormationData::PRECISION );
}

}

/*-------------------------------------------------------------------*/
/*!

 */
FormationData::FormationData()
{
    M_position_pairs.fill( 0 );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
FormationData::clear()
{
    M_role_names.fill( "" );
    M_position_pairs.fill( 0 );
    M_data_cont.clear();
    M_constraints.clear();
}

/*-------------------------------------------------------------------*/
/*!

 */
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
/*!

 */
int
FormationData::nearestDataIndex( const Vector2D & pos,
                                 const double & thr ) const
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
/*!

 */
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
/*!

 */
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
/*!

 */
bool
FormationData::existIntersectedConstraint( const Vector2D & pos ) const
{
    for ( const Constraint & c : M_constraints )
    {
        const Segment2D s( c.first->ball_, c.second->ball_ );

        if ( s.onSegmentWeakly( pos ) )
        {
            return true;
        }
    }

    return false;
}


/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationData::existIntersectedConstraints() const
{
    if ( M_constraints.empty() )
    {
        return false;
    }

    const size_t size = M_constraints.size();
    for ( size_t i = 0; i < size - 1; ++i )
    {
        const Constraint & c0 = M_constraints[i];
        const Segment2D s0( c0.first->ball_, c0.second->ball_ );

        for ( size_t j = i + 1; j < size; ++j )
        {
            const Constraint & c1 = M_constraints[j];

            if ( c0.first == c1.first
                 || c0.first == c1.second
                 || c0.second == c1.first
                 || c0.second == c1.second )
            {
                // shares same sample data
                continue;
            }

            if ( s0.existIntersectionExceptEndpoint( Segment2D( c1.first->ball_,
                                                                c1.second->ball_ ) ) )
            {
                return true;
            }
        }
    }

    return false;
}


/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationData::setRoleName( const int unum,
                            const std::string & name )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << "(FormationData;:setRoleName) illegal unum " << unum << std::endl;
        return false;
    }

    if ( name.empty() )
    {
        std::cerr << "(FormationData;:setRoleName) empty role name" << std::endl;
        return false;
    }

    M_role_names[unum - 1] = name;
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationData::setPositionPair( const int unum,
                                const int paired_unum )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << "(FormationData;:setPositionPair) ERROR: "
                  << "illegal unum " << unum << std::endl;
        return false;
    }

    if ( paired_unum < -1 || 11 < paired_unum )
    {
        std::cerr << "(FormationData;:setPositionPair) ERROR: "
                  << "illegal paired unum " << paired_unum << std::endl;
        return false;
    }

    if ( unum == paired_unum )
    {
        std::cerr << "(FormationData;:setPositionPair) ERROR: "
                  << "unum:" << unum << " = paired:" << paired_unum << std::endl;
        return false;
    }

    // check doubling registration
    for ( int i = 0; i < 11; ++i )
    {
        if ( i + 1 == unum
             || i + 1 == paired_unum )
        {
            continue;
        }

        if ( paired_unum == M_position_pairs[i] )
        {
            std::cerr << "(FormationData;:setPositionPair) ERROR: "
                      <<  paired_unum << " already registered "<< std::endl;
            return false;
        }
    }

    if ( 1 <= paired_unum && paired_unum <= 11
         && M_position_pairs[paired_unum - 1] > 0
         && M_position_pairs[paired_unum - 1] != unum )
    {
        std::cerr << "(FormationData;:setPositionPair) ERROR: "
                  << "unum=" << unum << ". paired_unum=" << paired_unum
                  << " already has the pair " << M_position_pairs[paired_unum - 1] << std::endl;
        return false;
    }

    M_position_pairs[unum - 1] = paired_unum;

    if ( 1 <= paired_unum && paired_unum <= 11 )
    {
        M_position_pairs[paired_unum - 1] = unum;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
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
    // check intersection with existing constraints
    //
    if ( existIntersectedConstraint( data.ball_ ) )
    {
        return std::string( "Exist intersected constraint" );
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
/*!

 */
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
    // check intersection with existing constraints
    //
    if ( existIntersectedConstraint( data.ball_ ) )
    {
        return std::string( "Exist intersected constraint" );
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
/*!

 */
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

    //
    // check intersection
    //
    if ( existIntersectedConstraints() )
    {
        *replaced = original_data;
        return std::string( "Exist intersected constraint" );
    }

    std::cerr << "Replaced data at index=" << idx << std::endl;

    //
    // update index value
    //
    updateDataIndex();

    return std::string();
}

/*-------------------------------------------------------------------*/
/*!

 */
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
    // remove constraints connected to the sample
    //
    const FormationData::Data * d = &(*it);

    Constraints::iterator c = M_constraints.begin();
    while ( c != M_constraints.end() )
    {
        if ( c->first == d
             || c->second == d )
        {
            c = M_constraints.erase( c );
        }
        else
        {
            ++c;
        }
    }

    //
    // remove sample
    //
    M_data_cont.erase( it );

    //
    updateDataIndex();

    return std::string();
}

/*-------------------------------------------------------------------*/
/*!

 */
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
/*!

 */
std::string
FormationData::addConstraint( const size_t origin_idx,
                              const size_t terminal_idx )
{
    //
    // validate index value
    //
    if ( origin_idx == terminal_idx )
    {
        std::cerr << "(FormationData::addConstraint) duplicated index"
                  << " first=" << origin_idx
                  << " second=" << terminal_idx
                  << std::endl;
        return std::string( "Duplicated index" );
    }

    if ( M_data_cont.size() < origin_idx + 1
         || M_data_cont.size() < terminal_idx + 1 )
    {
        std::cerr << "(FormationData::addConstraint) range over. data_size=" << M_data_cont.size()
                  << " first=" << origin_idx
                  << " second=" << terminal_idx
                  << std::endl;
        return std::string( "Invalid index" );
    }

    DataCont::iterator origin = M_data_cont.begin();
    DataCont::iterator terminal = M_data_cont.begin();
    std::advance( origin, origin_idx );
    std::advance( terminal, terminal_idx );

    //
    // check existing indices
    //
    {
        Constraints::value_type value( &(*origin), &(*terminal) );
        if ( std::find( M_constraints.begin(), M_constraints.end(), value ) != M_constraints.end() )
        {
            std::cerr << "(FormationData::addConstraint) the constraint is already registered. "
                      << " first=" << origin_idx
                      << " second=" << terminal_idx
                      << std::endl;
            return std::string( "Duplicated constraint" );
        }
    }
    {
        Constraints::value_type value( &(*terminal), &(*origin) );
        if ( std::find( M_constraints.begin(), M_constraints.end(), value ) != M_constraints.end() )
        {
            std::cerr << "(FormationData::addConstraint) the constraint is already registered. "
                      << " first=" << origin_idx
                      << " second=" << terminal_idx
                      << std::endl;
            return std::string( "Duplicated constraint" );
        }
    }

    //
    // check intersection with existing constraints
    //

    const Segment2D constraint( origin->ball_, terminal->ball_ );

    for ( const Constraint & c : M_constraints )
    {
        if ( constraint.existIntersectionExceptEndpoint( Segment2D( c.first->ball_,
                                                                    c.second->ball_ ) ) )
        {
            std::cerr << "(FormationData::addConstraint) the input constraint intersects with existing constraint. "
                      << " input:" << origin->ball_ << '-' << terminal->ball_
                      << " intersected:" << c.first->ball_ << '-' << c.second ->ball_
                      << std::endl;
            return std::string( "Exist intersected constraint" );
        }
    }

    //
    // check intersection with existing samples
    //
    for ( DataCont::const_iterator d = M_data_cont.begin();
          d != M_data_cont.end();
          ++d )
    {
        if ( d == origin
             || d == terminal )
        {
            continue;
        }

        if ( constraint.onSegmentWeakly( d->ball_ ) )
        {
            std::cerr <<"(FormationData::addConstraint) the input constraint intersects with existing sample. "
                      << " input:" << origin->ball_ << '-' << terminal->ball_
                      << " intersected:" << d->ball_ << '-' << d->ball_
                      << std::endl;
            return std::string( "Exist intersected constraint" );
        }
    }

    //
    // add to the container
    //
    M_constraints.push_back( Constraints::value_type( &(*origin), &(*terminal) ) );

    return std::string();
}

/*-------------------------------------------------------------------*/
/*!

 */
std::string
FormationData::replaceConstraint( const size_t idx,
                                  const size_t origin_idx,
                                  const size_t terminal_idx )
{
    if ( M_constraints.size() < idx + 1 )
    {
        std::cerr << __FILE__ << ':' << __LINE__ << ':'
                  << " replaceConstraint() range over. size=" << M_constraints.size()
                  << " index=" << idx
                  << std::endl;
        return std::string( "Invalid index" );
    }

    Constraints::iterator it = M_constraints.begin();
    std::advance( it, idx );

    Constraint backup = *it;
    it = M_constraints.erase( it );

    std::string err = addConstraint( origin_idx, terminal_idx );

    if ( ! err.empty() )
    {
        std::cerr << "(FormationData::replaceConstraint) could not replace the constraint."
                  << " index=" << idx
                  << std::endl;
        M_constraints.insert( it, backup );
        return err;
    }

    Constraint added = M_constraints.back();
    M_constraints.pop_back();
    M_constraints.insert( it, added );

    return std::string();
}

/*-------------------------------------------------------------------*/
/*!

 */
std::string
FormationData::removeConstraint( const size_t idx )
{
    if ( M_constraints.size() < idx + 1 )
    {
        std::cerr << __FILE__ << ':' << __LINE__ << ':'
                  << " removeConstraint() range over. size=" << M_constraints.size()
                  << " index=" << idx
                  << std::endl;
        return std::string( "Invalid index" );
    }

    Constraints::iterator it = M_constraints.begin();
    std::advance( it, idx );

    M_constraints.erase( it );
    return std::string();
}


/*-------------------------------------------------------------------*/
/*!

 */
std::string
FormationData::removeConstraint( const size_t origin_idx,
                                 const size_t terminal_idx )
{
    if ( M_data_cont.size() < origin_idx + 1
         || M_data_cont.size() < terminal_idx + 1 )
    {
        std::cerr << "(FormationData::removeConstraint) range over. data_size=" << M_data_cont.size()
                  << " first=" << origin_idx
                  << " second=" << terminal_idx
                  << std::endl;
        return std::string( "Invalid index" );
    }

    DataCont::iterator origin = M_data_cont.begin();
    DataCont::iterator terminal = M_data_cont.begin();
    std::advance( origin, origin_idx );
    std::advance( terminal, terminal_idx );

    {
        Constraints::value_type value( &(*origin), &(*terminal) );
        Constraints::iterator it = std::find( M_constraints.begin(), M_constraints.end(), value );
        if ( it != M_constraints.end() )
        {
            M_constraints.erase( it );
            return std::string();
        }
    }

    {
        Constraints::value_type value( &(*terminal), &(*origin) );
        Constraints::iterator it = std::find( M_constraints.begin(), M_constraints.end(), value );
        if ( it != M_constraints.end() )
        {
            M_constraints.erase( it );
            return std::string();
        }
    }

    std::cerr << "(FormationData::removeConstraint) no constraint (" << origin_idx << ',' << terminal_idx << ')' << std::endl;

    return std::string( "Invalid index" );
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationData::open( const std::string & filepath )
{
    std::ifstream fin( filepath.c_str() );
    if ( ! fin.is_open() )
    {
        std::cerr << "Failed to open the training data file [" << filepath << "]"
                  << std::endl;
        return false;
    }

    if ( ! read( fin ) )
    {
        std::cerr << "Failed to read the training data file [" << filepath << "]"
                  << std::endl;
        M_data_cont.clear();
        return false;
    }

    return true;
}


/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationData::read( std::istream & is )
{
    //return readOld( is );
    return readCSV( is );
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationData::readOld( std::istream & is )
{
    M_data_cont.clear();

    //
    // check header line.
    //

    int version = 0;
    int data_size = 0;

    std::string line_buf;
    int n_line = 0;
    while ( std::getline( is, line_buf ) )
    {
        ++n_line;
        if ( line_buf.empty()
             || line_buf[0] == '#'
             || ! line_buf.compare( 0, 2, "//" ) )
        {
            continue;
        }

        if ( ! line_buf.compare( 0, 13, "Begin Samples" ) )
        {
            int n_val = std::sscanf( line_buf.c_str(),
                                     " Begin Samples %d %d ",
                                     &version, &data_size );
            if ( n_val != 2 )
            {
                std::cerr << __FILE__ << ':' << __LINE__ << ':'
                          << " Illegal header [" << line_buf << "]"
                          << std::endl;
                return false;
            }
        }

        break;
    }

    //
    // read data.
    //
    bool success = true;

    if ( version >= 2 )
    {
        success = readV2( is, data_size );
    }
    else
    {
        // reset stream position.
        is.seekg( 0 );
        success = readV1( is );
    }

    if ( ! success )
    {
        clear();
    }

    return success;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationData::readCSV( std::istream & is )
{
    M_data_cont.clear();

    std::string line_buf;
    int n_data = 0;

    // // read the number of sample data
    {
        if ( ! std::getline( is, line_buf ) )
        {
            std::cerr << __FILE__ << ' ' << __LINE__ << ':'
                      << " ERROR Could not read any line." << std::endl;
            return false;
        }

        if ( std::sscanf( line_buf.c_str(), "Size,%d" , &n_data ) != 1 )
        {
            std::cerr << __FILE__ << ' ' << __LINE__ << ':'
                      << " ERROR Could not parse the number of data." << std::endl;
            return false;
        }
    }

    // skip header line
    {
        if ( ! std::getline( is, line_buf ) )
        {
            std::cerr << __FILE__ << ' ' << __LINE__ << ':'
                      << " ERROR no header line." << std::endl;
            return false;
        }
        if ( line_buf.compare( 0, 3, "idx" ) != 0 )
        {
            std::cerr << __FILE__ << ' ' << __LINE__ << ':'
                      << " ERROR Illegal header line." << std::endl;
            return false;
        }
    }

    // loop for reading all sample data
    for ( int idx = 0; idx < n_data; ++idx )
    {
        if ( ! std::getline( is, line_buf ) )
        {
            std::cerr << __FILE__ << ' ' << __LINE__ << ':'
                      << " ERROR Could not read the data line at idx=" << idx << std::endl;
            return false;
        }

        const char * buf = line_buf.c_str();
        int n_read = 0;

        int read_idx = 0;
        if ( std::sscanf( buf, "%d,%n", &read_idx, &n_read ) != 1 )
        {
            std::cerr << __FILE__ << ' ' << __LINE__ << ':'
                      << " ERROR Could not read the data index at idx=" << idx << std::endl;
            return false;
        }
        buf += n_read;

        FormationData::Data new_data;
        double read_x, read_y;

        // read ball
        if ( std::sscanf( buf, "%lf,%lf%n", &read_x, &read_y, &n_read ) != 2 )
        {
            std::cerr << __FILE__ << ' ' << __LINE__ << ':'
                      << " ERROR Could not read the ball data at idx=" << idx << std::endl;
            return false;
        }
        buf += n_read;

        new_data.ball_ = round_coordinates( read_x, read_y );

        // read players
        for ( int i = 1; i <= 11; ++i )
        {
            if ( std::sscanf( buf, ",%lf,%lf%n", &read_x, &read_y, &n_read ) != 2 )
            {
                std::cerr << __FILE__ << ' ' << __LINE__ << ':'
                          << " ERROR Could not read the player data " << i << " at idx=" << idx << std::endl;
                return false;
            }
            buf += n_read;

            new_data.players_.push_back( round_coordinates( read_x, read_y ) );
        }

        M_data_cont.push_back( new_data );
    }

    updateDataIndex();


    // TODO: read constraints

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationData::readV1( std::istream & is )
{
    std::string line_buf;

    int n_data = 0;
    while ( std::getline( is, line_buf ) )
    {
        ++n_data;

        std::istringstream istr( line_buf );
        double x, y;

        FormationData::Data new_data;

        istr >> x >> y;
        new_data.ball_ = round_coordinates( x, y );

        for ( int unum = 1; unum <= 11; ++unum )
        {
            if ( ! istr.good() )
            {
                std::cerr << __FILE__ << ':' << __LINE__ << ':'
                          << " *** ERROR *** Illegal player data."
                          << " data=" << n_data
                          << " unum=" << unum
                          << std::endl;
                return false;
            }

            istr >> x >> y;
            new_data.players_.push_back( round_coordinates( x, y ) );
        }

        M_data_cont.push_back( new_data );
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationData::readV2( std::istream & is,
                       const int data_size )
{
    if ( data_size < 0 )
    {
        std::cerr << "Formationdata::readV2()"
                  << " illegal data size =" << data_size
                  << std::endl;
        return false;
    }

    std::string line_buf;

    for ( int i = 0; i < data_size; ++i )
    {
        if ( ! readSample( is, i ) )
        {
            return false;
        }
    }

    updateDataIndex();

    //
    // read End tag
    //

    if ( ! std::getline( is, line_buf )
         || line_buf != "End Samples" )

    {
        std::cerr << __FILE__ << ':' << __LINE__ << ':'
                  << " *** ERROR *** readV2(). No end tag"
                  << " add 'End Samples' at the end of data."
                  << std::endl;
        return false;
    }

    //
    // read Constraints
    //

    if ( ! readConstraints( is ) )
    {
        return false;
    }

    return true;
}


/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationData::readSample( std::istream & is,
                           const int index )
{
    std::string line_buf;

    //
    // read index
    //
    while ( std::getline( is, line_buf ) )
    {
        if ( line_buf.empty()
             || line_buf[0] == '#'
             || ! line_buf.compare( 0, 2, "//" ) )
        {
            continue;
        }

        int read_index = -1;
        if ( std::sscanf( line_buf.c_str(),
                          "----- %d -----",
                          &read_index ) != 1
             || read_index != index )
        {
            std::cerr << " *** ERROR *** Illegal data segment. "
                      << " data=" << index
                      << '[' << line_buf << ']'
                      << std::endl;
            return false;
        }

        break;
    }

    //
    // read new sample data.
    //

    FormationData::Data new_data;

    double read_x = 0.0;
    double read_y = 0.0;

    //
    // read ball data
    //
    if ( ! std::getline( is, line_buf ) )
    {
        std::cerr << " *** ERROR *** failed to read ball data."
                  << " data=" << index
                  << std::endl;
        return false;
    }

    if ( std::sscanf( line_buf.c_str(),
                      " Ball %lf %lf ",
                      &read_x, &read_y ) != 2 )
    {
        std::cerr << " *** ERROR *** Illegal ball data."
                  << " data=" << index
                  << " [" << line_buf << "]"
                  << std::endl;
        return false;
    }

    new_data.ball_ = round_coordinates( read_x, read_y );

    //
    // read player data
    //

    int read_unum = 0;

    for ( int unum = 1; unum <= 11; ++unum )
    {
        if ( ! std::getline( is, line_buf ) )
        {
            std::cerr << " *** ERROR *** failed to read player data."
                      << " data=" << index
                      << " unum=" << unum
                      << std::endl;
            return false;
        }

        if ( std::sscanf( line_buf.c_str(),
                          " %d %lf %lf ",
                          &read_unum, &read_x, &read_y ) != 3
             || read_unum != unum )
        {
            std::cerr << __FILE__ << ':' << __LINE__ << ':'
                      << " *** ERROR *** Illegal player data."
                      << " data=" << index
                      << " unum=" << unum
                      << " [" << line_buf << "]"
                      << std::endl;
            return false;
        }

        new_data.players_.push_back( round_coordinates( read_x, read_y ) );
    }

    M_data_cont.push_back( new_data );

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationData::readConstraints( std::istream & is )
{
    std::string line_buf;

    //
    // read begin tag
    //
    std::streampos start_pos = is.tellg();

    if ( ! std::getline( is, line_buf ) )
    {
        // no constraint data
        return true;
    }

    if ( line_buf.compare( 0, std::strlen( "Begin Constraints" ), "Begin Constraints" ) != 0 )
    {
        is.seekg( start_pos );
        return true;
    }

    int constraints_size = 0;
    if ( std::sscanf( line_buf.c_str(),
                      " Begin Constraints %d ",
                      &constraints_size ) != 1 )
    {
        std::cerr << __FILE__ << ':' << __LINE__ << ':'
                  << " *** ERROR *** readConstraints(). Could not parse the size of constraints."
                  << " [" << line_buf << "]"
                  << std::endl;
        return false;
    }

    //
    // read constraints
    //
    for ( int i = 0; i < constraints_size; ++i )
    {
        if ( ! std::getline( is, line_buf ) )
        {
            std::cerr << __FILE__ << ':' << __LINE__ << ':'
                      << " readConstraints()"
                      << " Could not read a new line"
                      << std::endl;
            return false;
        }

        int idx0 = -1, idx1 = -1;
        if ( std::sscanf( line_buf.c_str(),
                          " %d %d ",
                          &idx0, &idx1 ) != 2 )
        {
            std::cerr << __FILE__ << ':' << __LINE__ << ':'
                      << " readConstraints()"
                      << " Illegal format. [" << line_buf << "]"
                      << std::endl;
            return false;
        }

        addConstraint( idx0, idx1 );
    }

    //
    // read end tag
    //
    if ( ! std::getline( is, line_buf )
         || line_buf != "End Constraints" )
    {
        std::cerr << __FILE__ << ':' << __LINE__ << ':'
                  << " readConstraints(). No end tag"
                  << " add 'End Constraints' at the end of constraints."
                  << std::endl;
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationData::save( const std::string & filepath ) const
{
    std::ofstream fout( filepath.c_str() );
    if ( ! fout.is_open() )
    {
        std::cerr << " *** ERROR *** failed to open the data file"
                  << " [" << filepath << "]"
                  << std::endl;
        return false;
    }

    if ( ! print( fout )  )
    {
        fout.close();
        std::cerr << " *** ERROR *** failed to save the data file"
                  << " [" << filepath << "]"
                  << std::endl;
        return false;
    }

    fout.close();

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationData::print( std::ostream & os ) const
{
    return printCSV( os );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationData::printOld( std::ostream & os ) const
{
    printV2( os );
    return os << std::flush;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationData::printV1( std::ostream & os ) const
{
    // put data to the stream
    for ( const FormationData::Data & data : M_data_cont )
    {
        os << round_coord( data.ball_.x ) << ' ' << round_coord( data.ball_.y ) << ' ';

        for ( const Vector2D & p : data.players_ )
        {
            os << round_coord( p.x ) << ' ' << round_coord( p.y ) << ' ';
        }
        os << '\n';
    }

    return os << std::flush;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationData::printV2( std::ostream & os ) const
{
   os << "Begin Samples 2 " << M_data_cont.size() << '\n';

    // put data to the stream
    int idx = 0;
    for ( const FormationData::Data & d : M_data_cont )
    {
        os << "----- " << idx << " -----\n";
        ++idx;
        os << "Ball " << round_coord( d.ball_.x ) << ' ' << round_coord( d.ball_.y ) << '\n';
        int unum = 1;
        for ( const Vector2D & p : d.players_ )
        {
            os << unum << ' ' << round_coord( p.x ) << ' ' << round_coord( p.y ) << '\n';
            ++unum;
        }
    }

    os << "End Samples\n";

    printConstraints( os );

    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationData::printConstraints( std::ostream & os ) const
{
    if ( M_constraints.empty() )
    {
        // no output
        return os;
    }

    os << "Begin Constraints " << M_constraints.size() << '\n';

    for ( const Constraint & c : M_constraints )
    {
        os << c.first->index_ << ' ' << c.second->index_ << '\n';
    }

    os << "End Constraints\n";

    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
FormationData::printCSV( std::ostream & os ) const
{
    os << "SampleData\n";
    os << "Size," << M_data_cont.size() << '\n';
    os << "idx,ballX,ballY";
    for ( int i = 1; i <= 11; ++i )
    {
        os << ',' << i << 'X'
           << ',' << i << 'Y';
    }
    os << '\n';

    int idx = 0;
    for ( const FormationData::Data & data : M_data_cont )
    {
        os << idx << ',';
        ++idx;
        os << round_coord( data.ball_.x ) << ',' << round_coord( data.ball_.y );
        for ( const Vector2D & p : data.players_ )
        {
            os << ',' << round_coord( p.x ) << ',' << round_coord( p.y );
        }
        os << '\n';
    }

    if ( ! M_constraints.empty() )
    {
         os << "Constraints," << M_constraints.size() << '\n';

         for ( const Constraint & c : M_constraints )
         {
             os << c.first->index_ << ',' << c.second->index_ << '\n';
         }
    }

    return os;
}

}
