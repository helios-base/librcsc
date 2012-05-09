// -*-c++-*-

/*!
  \file sample_data.h
  \brief formation sample data class Header File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
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

#ifndef RCSC_FORMATION_SAMPLE_DATA_H
#define RCSC_FORMATION_SAMPLE_DATA_H

#include <rcsc/geom/vector_2d.h>

#include <boost/shared_ptr.hpp>

#include <vector>
#include <list>
#include <utility>
#include <string>
#include <iostream>

namespace rcsc {

class Formation;

namespace formation {

/*!
  \struct SampleData
  \brief training data type.
*/
struct SampleData {

    static const double PRECISION; //!< coordinates value precision
    typedef std::vector< Vector2D > PlayerCont; //!< player data container type

    int index_;
    Vector2D ball_; //!< ball position
    PlayerCont players_; //!< players' position

    /*!
      \brief default constructor
    */
    SampleData()
        : index_( -1 )
      {
          players_.reserve( 11 );
      }

    /*!
      \brief construct with all data
      \param ball ball position
      \param players players' position
    */
    SampleData( const Vector2D & ball,
                const PlayerCont & players )
        : index_( -1 )
        , ball_( ball )
        , players_( players )
      { }

    /*!
      \brief get the position of specified player.
      \param unum player number [1..11].
      \return position value.
    */
    Vector2D getPosition( const int unum ) const
      {
          return players_.at( unum - 1 );
      }
};


/*!
  \class SampleDataSet
  \brief sample data set for formation training
 */
class SampleDataSet {
public:

    typedef boost::shared_ptr< SampleDataSet > Ptr; //!< shared pointer type.
    typedef boost::shared_ptr< const SampleDataSet > ConstPtr; //!< shared const pointer type.

    typedef std::list< SampleData > DataCont; //!< data container type.
    typedef std::pair< size_t, const SampleData * > IndexData; //!< index & data pair type.
    typedef std::pair< const SampleData *, const SampleData * > Constraint; //!< constraint type
    typedef std::vector< Constraint > Constraints; //!< constraint container type.

    /*!
      \enum ErrorType
      \brief error type for data operation.
     */
    enum ErrorType {
        NO_FORMATION,
        TOO_MANY_DATA,
        TOO_NEAR_DATA,
        ILLEGAL_SYMMETRY_DATA,
        TOO_NEAR_SYMMETRY_DATA,
        INSERT_RANGE_OVER,
        INVALID_INDEX,
        DUPLICATED_INDEX,
        DUPLICATED_CONSTRAINT,
        INTERSECTS_CONSTRAINT,
        NO_ERROR,
    };

    static const size_t MAX_DATA_SIZE; //!< max data size
    static const double NEAR_DIST_THR; //!< data distance threshold

private:

    DataCont M_data_cont; //!< data container.
    Constraints M_constraints; //!< constraint container.

public:

    /*!
      \brief default constructor.
     */
    SampleDataSet();

    /*!
      \brief copy constructor.
      \param other source object.
     */
    SampleDataSet( const SampleDataSet & other )
        : M_data_cont( other.dataCont() )
      { }

    /*!
      \brief substitution operator.
      \param other source object.
     */
    const SampleDataSet & operator=( const SampleDataSet & other )
      {
          if ( this != &other )
          {
              M_data_cont = other.dataCont();
          }
          return *this;
      }

    /*!
      \brief virtual destructor.
     */
    virtual
    ~SampleDataSet()
      { }

    /*!
      \brief clear all data.
     */
    void clear();

    /*!
      \brief get the data container.
      \return data container.
     */
    const DataCont & dataCont() const
      {
          return M_data_cont;
      }

    const Constraints & constraints() const
      {
          return M_constraints;
      }

    /*!
      \brief get the specified index data.
      \return const pointer to the data. if no matched data, NULL is returned.
     */
    const SampleData * data( const size_t idx ) const;

    /*!
      \brief get the data nearest to the input point.
      \param pos input point.
      \param dist_thr distance threshold
      \return data nearest to the input point
     */
    IndexData nearestData( const Vector2D & pos,
                           const double & dist_thr ) const;

    /*!
      \brief check if there are exsiting data near to input data.
      \param data input data.
      \return checked result.
     */
    bool existTooNearData( const SampleData & data ) const;

private:
    /*!
      \brief update index value of all data.
     */
    void updateDataIndex();

    /*!
      \brief check if there are constraints intersected wht the input position.
      \param pos input position.
      \param checked result.
     */
    bool existIntersectedConstraint( const Vector2D & pos ) const;

    /*!
      \brief check if there are constraints intersected with others.
      \param checked result.
     */
    bool existIntersectedConstraints() const;

public:

    /*!
      \brief append new data.
      \param formation formation is needed to check the symmetry data.
      \param data input data.
      \param symmetry switch variable to determine symmetry mode.
      \return error code.
     */
    ErrorType addData( const Formation & formation,
                       const SampleData & data,
                       const bool symmetry );

    /*!
      \brief insert new data just before the input index.
      \param formation formation is needed to check the symmetry data.
      \param idx input index.
      \param data input data.
      \param symmetry switch variable to determine symmetry mode.
      \return error code.
     */
    ErrorType insertData( const Formation & formation,
                          const size_t idx,
                          const SampleData & data,
                          const bool symmetry );

    /*!
      \brief replace exsiting data at input index with input data.
      \param formation formation is needed to check the symmetry data.
      \param idx input index.
      \param data input data.
      \param symmetry switch variable to determine symmetry mode.
      \return error code.
     */
    ErrorType replaceData( const Formation & formation,
                           const size_t idx,
                           const SampleData & data,
                           const bool symmetry );

    /*!
      \brief delete exsiting data at input index.
      \param idx input index.
      \return error code.
     */
    ErrorType removeData( const size_t idx );

   /*!
      \brief move the specified data to new index position.
      \param old_idx old index.
      \param new_idx new index.
      \return error code.
     */
    ErrorType changeDataIndex( const size_t old_idx,
                               const size_t new_idx );

private:

    /*!
      \brief replace exsiting data at input index with input data.
      \param formation formation is needed to check the symmetry data.
      \param idx input index.
      \param data input data.
      \param symmetry switch variable to determine symmetry mode.
      \return error code.
     */
    ErrorType replaceSymmetryData( const Formation & formation,
                                   const SampleData & original_data,
                                   const SampleData & reversed_data );

public:

    /*!
      \brief add new constraint between given indices.
      \param origin_idx index of first vertex.
      \param terminal_idx index of second vertex.
      \return error code.
     */
    ErrorType addConstraint( const size_t origin_idx,
                             const size_t terminal_idx );

    /*!
      \brief delete the specified constraint.
      \param idx replaced index.
      \param origin_idx new origin index.
      \param terminal_idx new terminal index.
      \return error code.
     */
    ErrorType replaceConstraint( const size_t idx,
                                 const size_t origin_idx,
                                 const size_t terminal_idx );

    /*!
      \brief delete the specified constraint.
      \param idx index of removed constraint.
      \return error code.
     */
    ErrorType removeConstraint( const size_t idx );

    /*!
      \brief delete the specified constraint.
      \param origin_idx origin index of removed constraint.
      \param terminal_idx terminal index of removed constraint.
      \return error code.
     */
    ErrorType removeConstraint( const size_t origin_idx,
                                const size_t terminal_idx );


    /*!
      \brief reverse Y coodinate value of given positions.
      \param formation formation instance.
      \param positions reversed value container.
     */
    void reverseY( const Formation & formation,
                   SampleData::PlayerCont & positions ) const;

    /*!
      \brief open the file and read data from it.
      \param filepath file path string.
      \return result status.
     */
    bool open( const std::string & filepath );

    /*!
      \brief read data from input stream.
      \param is reference to the input stream.
      \return result status.
     */
    bool read( std::istream & is );

private:
    /*!
      \brief read data from input stream (v1 format).
      \param is reference to the input stream.
      \return result status.
     */
    bool readOld( std::istream & is );

    /*!
      \brief read data from input stream (v2 format).
      \param is reference to the input stream.
      \return result status.
     */
    bool readV2( std::istream & is,
                 const int data_size );

    bool readSample( std::istream & is,
                     const int index );
    bool readConstraints( std::istream & is );

public:

    /*!
      \brief save data to the file.
      \param filepath file path string.
      \return result status.
     */
    bool save( const std::string & filepath ) const;

    /*!
      \brief print data to the output stream.
      \param os reference to the output stream.
      \return reference to the output stream.
     */
    std::ostream & print( std::ostream & os ) const;
private:

    /*!
      \brief print data to the output stream (v1 format).
      \param os reference to the output stream.
      \return reference to the output stream.
     */
    std::ostream & printOld( std::ostream & os ) const;

    /*!
      \brief print data to the output stream (v2 format).
      \param os reference to the output stream.
      \return reference to the output stream.
     */
    std::ostream & printV2( std::ostream & os ) const;

    std::ostream & printConstraints( std::ostream & os ) const;
};

}
}

#endif
