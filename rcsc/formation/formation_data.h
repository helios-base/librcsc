// -*-c++-*-

/*!
  \file formation_data.h
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

#ifndef RCSC_FORMATION_FORMATION_DATA_H
#define RCSC_FORMATION_FORMATION_DATA_H

#include <rcsc/geom/vector_2d.h>

#include <memory>
#include <array>
#include <vector>
#include <list>
#include <utility>
#include <string>
#include <iostream>

namespace rcsc {

/*!
  \class FormationData
  \brief data for constructing the formation
*/
class FormationData {
public:

    /*!
      \struct Data
      \brief training data element.
    */
    struct Data {
        int index_;
        Vector2D ball_; //!< ball position
        std::vector< Vector2D > players_; //!< players' position

        /*!
          \brief default constructor
        */
        Data()
            : index_( -1 )
        {
            players_.reserve( 11 );
        }

        /*!
          \brief construct with all data
          \param ball ball position
          \param players players' position
        */
        Data( const Vector2D & ball,
              const std::vector< Vector2D > & players )
            : index_( -1 ),
              ball_( ball ),
              players_( players )
        { }

        /*!
          \brief get the position of specified player.
          \param num player number [1..11].
          \return position value.
        */
        Vector2D getPosition( const int num ) const
        {
            return players_.at( num - 1 );
        }
    };

    typedef std::shared_ptr< FormationData > Ptr;
    typedef std::shared_ptr< const FormationData > ConstPtr;
    typedef std::list< Data > DataCont; //!< data container type.

    static const double PRECISION; //!< coordinates value precision
    static const size_t MAX_DATA_SIZE; //!< max data size
    static const double NEAR_DIST_THR; //!< data distance threshold

    static double round_xy( const double xy );
    static rcsc::Vector2D rounded_vector( const double x,
                                          const double y );

private:

    DataCont M_data_cont; //!< data container.

    // not used
    FormationData( const FormationData & other ) = delete;
    const FormationData & operator=( const FormationData & other ) = delete;

public:

    /*!
      \brief default constructor.
    */
    FormationData();


    /*!
      \brief virtual destructor.
    */
    virtual
    ~FormationData()
    { }

    /*!
      \brief clear all data.
    */
    void clear();

    // /*!
    //   \brief create default data.
    //  */
    // void createDefault();

    /*!
      \brief get the data container.
      \return data container.
    */
    const DataCont & dataCont() const
    {
        return M_data_cont;
    }

    /*!
      \brief get the specified index data.
      \return const pointer to the data. if no matched data, NULL is returned.
    */
    const Data * data( const size_t idx ) const;

    /*!
      \brief get the data index nearest to the input point.
      \param pos input point.
      \param dist_thr distance threshold
      \return index of the result data. -1 if no result
    */
    int nearestDataIndex( const Vector2D & pos,
                          const double dist_thr ) const;

    /*!
      \brief check if there are exsiting data near to input data.
      \param data input data.
      \return checked result.
    */
    bool existTooNearData( const Data & data ) const;

private:
    /*!
      \brief update index value of all data.
    */
    void updateDataIndex();

public:

    /*!
      \brief append new data.
      \param data new data.
      \return error message if error occurd. otherwise, empty string.
    */
    std::string addData( const Data & data );

    /*!
      \brief insert new data just before the input index.
      \param idx target index.
      \param data new data.
      \return error message if error occurd. otherwise, empty string.
    */
    std::string insertData( const size_t idx,
                            const Data & data );

    /*!
      \brief replace exsiting data at input index with input data.
      \param idx target index.
      \param data new data.
      \return error message if error occurd. otherwise, empty string.
    */
    std::string replaceData( const size_t idx,
                             const Data & data );

    /*!
      \brief delete exsiting data at input index.
      \param idx input index.
      \return error message if error occurd. otherwise, empty string.
    */
    std::string removeData( const size_t idx );

    /*!
      \brief move the specified data to new index position.
      \param old_idx old index.
      \param new_idx new index.
      \return error message if error occurd. otherwise, empty string.
    */
    std::string changeDataIndex( const size_t old_idx,
                                 const size_t new_idx );

    /*!
      \brief print data in JSON format
      \param os output stream.
      \return output stream.
    */
    std::ostream & print( std::ostream & os ) const;

    /*!
      \brief read data in JSON format
      \param is input stream.
      \return input stream.
    */
    bool read( std::istream & is );
};

}

#endif
