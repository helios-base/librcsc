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

#include "sample_data.h"

#include "formation.h"

#include <rcsc/geom/segment_2d.h>

#include <iterator>
#include <algorithm>
#include <limits>
#include <fstream>
#include <sstream>

namespace rcsc {
namespace formation {

const double SampleData::PRECISION = 0.01;

const size_t SampleDataSet::MAX_DATA_SIZE = 128;
const double SampleDataSet::NEAR_DIST_THR = 0.5;

namespace {

inline
double
round_coord( const double & val )
{
    return rint( val / SampleData::PRECISION ) * SampleData::PRECISION;
}

inline
Vector2D
round_coordinates( const double & x,
                   const double & y )
{
    return Vector2D( rint( x / SampleData::PRECISION ) * SampleData::PRECISION,
                     rint( y / SampleData::PRECISION ) * SampleData::PRECISION );
}

}

/*-------------------------------------------------------------------*/
/*!

 */
SampleDataSet::SampleDataSet()
    : M_data_cont()
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
SampleDataSet::clear()
{
    M_data_cont.clear();
    M_constraints.clear();
}

/*-------------------------------------------------------------------*/
/*!

 */
const
SampleData *
SampleDataSet::data( const size_t idx ) const
{
    if ( M_data_cont.empty()
         || M_data_cont.size() < idx )
    {
        return static_cast< const SampleData * >( 0 );
    }

    SampleDataSet::DataCont::const_iterator it = M_data_cont.begin();
    std::advance( it, idx );

    return &(*it);
}

/*-------------------------------------------------------------------*/
/*!

 */
SampleDataSet::IndexData
SampleDataSet::nearestData( const Vector2D & pos,
                            const double & thr ) const
{
    const double dist_thr2 = thr * thr;

    std::pair< size_t, const SampleData * > rval( size_t( -1 ), static_cast< SampleData * >( 0 ) );

    size_t index = 0;
    double min_dist2 = std::numeric_limits< double >::max();

    const DataCont::const_iterator p_end = M_data_cont.end();
    for ( DataCont::const_iterator p = M_data_cont.begin();
          p != p_end;
          ++p, ++index )
    {
        double d2 = p->ball_.dist2( pos );
        if ( d2 < dist_thr2
             && d2 < min_dist2 )
        {
            min_dist2 = d2;
            rval.first = index;
            rval.second = &(*p);
            break;
        }
    }

    return rval;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
SampleDataSet::existTooNearData( const SampleData & data ) const
{
    const double dist_thr2 = NEAR_DIST_THR * NEAR_DIST_THR;

    const DataCont::const_iterator end = M_data_cont.end();
    for ( DataCont::const_iterator d = M_data_cont.begin();
          d != end;
          ++d )
    {
        if ( d->ball_.dist2( data.ball_ ) < dist_thr2 )
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
SampleDataSet::updateDataIndex()
{
    int index = 0;
    for ( DataCont::iterator d = M_data_cont.begin();
          d != M_data_cont.end();
          ++d, ++index )
    {
        d->index_ = index;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
SampleDataSet::existIntersectedConstraint( const Vector2D & pos ) const
{
    for ( Constraints::const_iterator c = M_constraints.begin();
          c != M_constraints.end();
          ++c )
    {
        const Segment2D s( c->first->ball_, c->second->ball_ );

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
SampleDataSet::existIntersectedConstraints() const
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
SampleDataSet::ErrorType
SampleDataSet::addData( const Formation & formation,
                        const SampleData & data,
                        const bool symmetry )
{
    if ( M_data_cont.size() >= MAX_DATA_SIZE )
    {
        return TOO_MANY_DATA;
    }

    if ( existTooNearData( data ) )
    {
        return TOO_NEAR_DATA;
    }

    //
    // check intersection with existing constraints
    //
    if ( existIntersectedConstraint( data.ball_ ) )
    {
        return INTERSECTS_CONSTRAINT;
    }

    //
    // add data
    //
    M_data_cont.push_back( data );

    std::cerr << "Added data. current data size = " << M_data_cont.size()
              << std::endl;

    //
    // add symmetry data
    //
    if ( symmetry )
    {
        if ( data.ball_.absY() < 0.5 )
        {
            //return ILLEGAL_SYMMETRY_DATA;
            return NO_ERROR;
        }

        SampleData reversed = data;
        reversed.ball_.y *= -1.0;
        reverseY( formation, reversed.players_ );

        return addData( formation, reversed, false );
    }

    updateDataIndex();

    return NO_ERROR;
}

/*-------------------------------------------------------------------*/
/*!

 */
SampleDataSet::ErrorType
SampleDataSet::insertData( const Formation & formation,
                           const size_t idx,
                           const SampleData & data,
                           const bool symmetry )
{
    if ( M_data_cont.size() >= MAX_DATA_SIZE )
    {
        return TOO_MANY_DATA;
    }

    if ( M_data_cont.size() < idx )
    {
        return INSERT_RANGE_OVER;
    }

    if ( existTooNearData( data ) )
    {
        return TOO_NEAR_DATA;
    }

    //
    // check intersection with existing constraints
    //
    if ( existIntersectedConstraint( data.ball_ ) )
    {
        return INTERSECTS_CONSTRAINT;
    }

    //
    // insert data
    //
    DataCont::iterator it = M_data_cont.begin();
    std::advance( it, idx );

    M_data_cont.insert( it, data );

    std::cerr << "Inserted data at index="
              << std::distance( M_data_cont.begin(), it ) + 1
              << ". current data size = "
              << M_data_cont.size()
              << std::endl;

    //
    // insert symmetry data
    //
    if ( symmetry )
    {
        if ( data.ball_.absY() < 0.5 )
        {
            //return ILLEGAL_SYMMETRY_DATA;
            return NO_ERROR;
        }

        SampleData reversed = data;
        reversed.ball_.y *= -1.0;
        reverseY( formation, reversed.players_ );

        return insertData( formation, idx + 1, reversed, false );
    }

    updateDataIndex();

    return NO_ERROR;
}

/*-------------------------------------------------------------------*/
/*!

 */
SampleDataSet::ErrorType
SampleDataSet::replaceData( const Formation & formation,
                            const size_t idx,
                            const SampleData & data,
                            const bool symmetry )
{
    if ( M_data_cont.size() < idx )
    {
        return INVALID_INDEX;
    }

    DataCont::iterator replaced = M_data_cont.begin();
    std::advance( replaced, idx );

    //
    // check near adata
    //
    {
        const double dist_thr2 = NEAR_DIST_THR * NEAR_DIST_THR;
        const DataCont::iterator end = M_data_cont.end();
        for ( DataCont::iterator it = M_data_cont.begin();
              it != end;
              ++it )
        {
            if ( it != replaced
                 && it->ball_.dist2( data.ball_ ) < dist_thr2 )
            {
                return TOO_NEAR_DATA;
            }
        }
    }

    SampleData original_data = *replaced;
    *replaced = data;

    //
    // check intersection
    //
    if ( existIntersectedConstraints() )
    {
        *replaced = original_data;
        return INTERSECTS_CONSTRAINT;
    }

    std::cerr << "Replaced data at index=" << idx
              << std::endl;

    //
    // replace symmetry data
    //
    if ( symmetry )
    {
        if ( data.ball_.absY() < 0.5 )
        {
            return NO_ERROR;
        }

        SampleData reversed = data;
        reversed.ball_.y *= -1.0;
        reverseY( formation, reversed.players_ );

        return replaceSymmetryData( formation, original_data, reversed );
    }

    //
    // update index value
    //
    updateDataIndex();

    return NO_ERROR;
}

/*-------------------------------------------------------------------*/
/*!

 */
SampleDataSet::ErrorType
SampleDataSet::replaceSymmetryData( const Formation & formation,
                                    const SampleData & original_data,
                                    const SampleData & reversed_data )
{
    if ( reversed_data.ball_.absY() < 0.5 )
    {
        return ILLEGAL_SYMMETRY_DATA;
    }

    //
    // check near data.
    //

    DataCont::iterator replaced = M_data_cont.end();
    {
        double min_dist2 = std::numeric_limits< double >::max();
        const Vector2D pos( original_data.ball_.x, - original_data.ball_.y );
        const double dist_thr2 = NEAR_DIST_THR * NEAR_DIST_THR;

        const DataCont::iterator end = M_data_cont.end();
        for ( DataCont::iterator it = M_data_cont.begin();
              it != end;
              ++it )
        {
            double d2 = it->ball_.dist2( pos );
            if ( d2 < dist_thr2
                 && d2 < min_dist2 )
            {
                min_dist2 = d2;
                replaced = it;
            }
        }
    }

    //
    // not found
    //
    if ( replaced == M_data_cont.end() )
    {
        // try to add new data
        return addData( formation, reversed_data, false );
    }

    //
    // found
    //

    SampleData tmp = *replaced;
    *replaced = reversed_data;

    //
    // check intersection
    //
    if ( existIntersectedConstraints() )
    {
        *replaced = tmp;
        return INTERSECTS_CONSTRAINT;
    }

    std::cerr << "Replaced symmetry data at index="
              << std::distance( M_data_cont.begin(), replaced ) + 1
              << std::endl;

    return NO_ERROR;
}

/*-------------------------------------------------------------------*/
/*!

 */
SampleDataSet::ErrorType
SampleDataSet::removeData( const size_t idx )
{
    if ( M_data_cont.size() < idx )
    {
        return INVALID_INDEX;
    }

    DataCont::iterator it = M_data_cont.begin();
    std::advance( it, idx );

    //
    // remove constraints connected to the sample
    //
    const SampleData * d = &(*it);

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

    return NO_ERROR;
}

/*-------------------------------------------------------------------*/
/*!

 */
SampleDataSet::ErrorType
SampleDataSet::changeDataIndex( const size_t old_idx,
                                const size_t new_idx )
{
    if ( old_idx == new_idx
         || M_data_cont.size() < old_idx
         || M_data_cont.size() < new_idx )
    {
        return INVALID_INDEX;
    }

    DataCont::iterator oit = M_data_cont.begin();
    std::advance( oit, old_idx );

    DataCont::iterator nit = M_data_cont.begin();
    std::advance( nit, new_idx );

    DataCont tmp_list;
    tmp_list.splice( tmp_list.end(), M_data_cont, oit );
    M_data_cont.splice( nit, tmp_list );

    updateDataIndex();

    return NO_ERROR;
}

/*-------------------------------------------------------------------*/
/*!

 */
SampleDataSet::ErrorType
SampleDataSet::addConstraint( const size_t origin_idx,
                              const size_t terminal_idx )
{
    //
    // validate index value
    //
    if ( origin_idx == terminal_idx )
    {
        std::cerr << __FILE__ << ':' << __LINE__ << ':'
                  << " addConstraint() duplicated index"
                  << " first=" << origin_idx
                  << " second=" << terminal_idx
                  << std::endl;
        return DUPLICATED_INDEX;
    }

    if ( M_data_cont.size() < origin_idx + 1
         || M_data_cont.size() < terminal_idx + 1 )
    {
        std::cerr << __FILE__ << ':' << __LINE__ << ':'
                  << " addConstraint() range over. data_size=" << M_data_cont.size()
                  << " first=" << origin_idx
                  << " second=" << terminal_idx
                  << std::endl;
        return INVALID_INDEX;
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
            std::cerr << __FILE__ << ':' << __LINE__ << ':'
                      << " addConstraint() the constraint is already registered. "
                      << " first=" << origin_idx
                      << " second=" << terminal_idx
                      << std::endl;
            return DUPLICATED_CONSTRAINT;
        }
    }
    {
        Constraints::value_type value( &(*terminal), &(*origin) );
        if ( std::find( M_constraints.begin(), M_constraints.end(), value ) != M_constraints.end() )
        {
            std::cerr << __FILE__ << ':' << __LINE__ << ':'
                      << " addConstraint() the constraint is already registered. "
                      << " first=" << origin_idx
                      << " second=" << terminal_idx
                      << std::endl;
            return DUPLICATED_CONSTRAINT;
        }
    }

    //
    // check intersection with existing constraints
    //

    const Segment2D constraint( origin->ball_, terminal->ball_ );

    for ( Constraints::const_iterator c = M_constraints.begin();
          c != M_constraints.end();
          ++c )
    {
        if ( constraint.existIntersectionExceptEndpoint( Segment2D( c->first->ball_,
                                                                    c->second->ball_ ) ) )
        {
            std::cerr << __FILE__ << ':' << __LINE__ << ':'
                      << " addConstraint() the input constraint intersects with existing constraint. "
                      << " input:" << origin->ball_ << '-' << terminal->ball_
                      << " intersected:" << c->first->ball_ << '-' << c->second ->ball_
                      << std::endl;
            return INTERSECTS_CONSTRAINT;
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
            std::cerr << __FILE__ << ':' << __LINE__ << ':'
                      << " addConstraint() the input constraint intersects with existing sample. "
                      << " input:" << origin->ball_ << '-' << terminal->ball_
                      << " intersected:" << d->ball_ << '-' << d->ball_
                      << std::endl;
            return INTERSECTS_CONSTRAINT;
        }
    }

    //
    // add to the container
    //
    M_constraints.push_back( Constraints::value_type( &(*origin), &(*terminal) ) );

    return NO_ERROR;
}

/*-------------------------------------------------------------------*/
/*!

 */
SampleDataSet::ErrorType
SampleDataSet::replaceConstraint( const size_t idx,
                                  const size_t origin_idx,
                                  const size_t terminal_idx )
{
    if ( M_constraints.size() < idx + 1 )
    {
        std::cerr << __FILE__ << ':' << __LINE__ << ':'
                  << " replaceConstraint() range over. size=" << M_constraints.size()
                  << " index=" << idx
                  << std::endl;
        return INVALID_INDEX;
    }

    Constraints::iterator it = M_constraints.begin();
    std::advance( it, idx );

    Constraint backup = *it;
    it = M_constraints.erase( it );

    ErrorType err = addConstraint( origin_idx, terminal_idx );

    if ( err != NO_ERROR )
    {
        std::cerr << __FILE__ << ':' << __LINE__ << ':'
                  << " replaceConstraint() could not replace the constraint."
                  << " index=" << idx
                  << std::endl;
        M_constraints.insert( it, backup );
        return err;
    }

    Constraint added = M_constraints.back();
    M_constraints.pop_back();
    M_constraints.insert( it, added );

    return NO_ERROR;
}

/*-------------------------------------------------------------------*/
/*!

 */
SampleDataSet::ErrorType
SampleDataSet::removeConstraint( const size_t idx )
{
    if ( M_constraints.size() < idx + 1 )
    {
        std::cerr << __FILE__ << ':' << __LINE__ << ':'
                  << " removeConstraint() range over. size=" << M_constraints.size()
                  << " index=" << idx
                  << std::endl;
        return INVALID_INDEX;
    }

    Constraints::iterator it = M_constraints.begin();
    std::advance( it, idx );

    M_constraints.erase( it );
    return NO_ERROR;
}


/*-------------------------------------------------------------------*/
/*!

 */
SampleDataSet::ErrorType
SampleDataSet::removeConstraint( const size_t origin_idx,
                                 const size_t terminal_idx )
{
    if ( M_data_cont.size() < origin_idx + 1
         || M_data_cont.size() < terminal_idx + 1 )
    {
        std::cerr << __FILE__ << ':' << __LINE__ << ':'
                  << " removeConstraint() range over. data_size=" << M_data_cont.size()
                  << " first=" << origin_idx
                  << " second=" << terminal_idx
                  << std::endl;
        return INVALID_INDEX;
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
            return NO_ERROR;
        }
    }

    {
        Constraints::value_type value( &(*terminal), &(*origin) );
        Constraints::iterator it = std::find( M_constraints.begin(), M_constraints.end(), value );
        if ( it != M_constraints.end() )
        {
            M_constraints.erase( it );
            return NO_ERROR;
        }
    }

    std::cerr << __FILE__ << ':' << __LINE__ << ':'
              << " removeConstraint() no constraint (" << origin_idx << ',' << terminal_idx << ')'
              << std::endl;

    return INVALID_INDEX;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SampleDataSet::reverseY( const Formation & formation,
                         SampleData::PlayerCont & positions ) const

{
    SampleData::PlayerCont old_positions = positions;

    int unum = 1;
    for ( SampleData::PlayerCont::iterator it = positions.begin();
          it != positions.end();
          ++it, ++unum )
    {
        if ( formation.isCenterType( unum ) )
        {
            it->y *= -1.0;
        }
        else if ( formation.isSymmetryType( unum ) )
        {
            int symmetry_unum = formation.getSymmetryNumber( unum );
            if ( symmetry_unum == 0 ) continue;
            it->x = old_positions[symmetry_unum - 1].x;
            it->y = old_positions[symmetry_unum - 1].y * -1.0;
        }
        else if ( formation.isSideType( unum ) )
        {
            it->y *= -1.0;
            for ( int iunum = 1; iunum <= 11; ++iunum )
            {
                if ( formation.getSymmetryNumber( iunum ) == unum )
                {
                    it->x = old_positions[iunum - 1].x;
                    it->y = old_positions[iunum - 1].y * -1.0;
                }
            }
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
SampleDataSet::open( const std::string & filepath )
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
SampleDataSet::read( std::istream & is )
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
        success = readOld( is );
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
SampleDataSet::readOld( std::istream & is )
{
    std::string line_buf;

    int n_data = 0;
    while ( std::getline( is, line_buf ) )
    {
        ++n_data;

        std::istringstream istr( line_buf );
        double x, y;

        SampleData new_data;

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
SampleDataSet::readV2( std::istream & is,
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
SampleDataSet::readSample( std::istream & is,
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

    SampleData new_data;

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
SampleDataSet::readConstraints( std::istream & is )
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
SampleDataSet::save( const std::string & filepath ) const
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
SampleDataSet::print( std::ostream & os ) const
{
    printV2( os );
    return os << std::flush;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SampleDataSet::printOld( std::ostream & os ) const
{
    // put data to the stream
    for ( DataCont::const_iterator it = M_data_cont.begin();
          it != M_data_cont.end();
          ++it )
    {
        os << round_coord( it->ball_.x ) << ' ' << round_coord( it->ball_.y ) << ' ';

        for ( std::vector< Vector2D >::const_iterator p = it->players_.begin();
              p != it->players_.end();
              ++p )
        {
            os << round_coord( p->x ) << ' ' << round_coord( p->y ) << ' ';
        }
        os << '\n';
    }

    return os << std::flush;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SampleDataSet::printV2( std::ostream & os ) const
{
   os << "Begin Samples 2 " << M_data_cont.size() << '\n';

    // put data to the stream
    int idx = 0;
    for ( DataCont::const_iterator it = M_data_cont.begin();
          it != M_data_cont.end();
          ++it, ++idx )
    {
        os << "----- " << idx << " -----\n";
        os << "Ball " << round_coord( it->ball_.x ) << ' ' << round_coord( it->ball_.y ) << '\n';
        int unum = 1;
        for ( std::vector< Vector2D >::const_iterator p = it->players_.begin();
              p != it->players_.end();
              ++p, ++unum )
        {
            os << unum << ' ' << round_coord( p->x ) << ' ' << round_coord( p->y ) << '\n';
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
SampleDataSet::printConstraints( std::ostream & os ) const
{
    if ( M_constraints.empty() )
    {
        // no output
        return os;
    }

    os << "Begin Constraints " << M_constraints.size() << '\n';

    for ( Constraints::const_iterator it = M_constraints.begin();
          it != M_constraints.end();
          ++it )
    {
        os << it->first->index_ << ' ' << it->second->index_ << '\n';
    }

    os << "End Constraints\n";

    return os;
}

}
}
