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
          \param unum player number [1..11].
          \return position value.
        */
        Vector2D getPosition( const int unum ) const
        {
            return players_.at( unum - 1 );
        }
    };

    typedef std::shared_ptr< FormationData > Ptr;
    typedef std::shared_ptr< const FormationData > ConstPtr;
    typedef std::list< Data > DataCont; //!< data container type.
    typedef std::pair< const Data *, const Data * > Constraint; //!< constraint type
    typedef std::vector< Constraint > Constraints; //!< constraint container type.

    static const double PRECISION; //!< coordinates value precision
    static const size_t MAX_DATA_SIZE; //!< max data size
    static const double NEAR_DIST_THR; //!< data distance threshold

private:

    //! role name string
    std::array< std::string, 11 > M_role_names;

    //! uniform number of the paired player. 0 means no pair.
    std::array< int, 11 > M_position_pairs;

    DataCont M_data_cont; //!< data container.
    Constraints M_constraints; //!< constraint container.

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

    /*!
      \brief get the role name array
      \return const refrence to the array instance
     */
    const std::array< std::string, 11 > & roleNames() const
    {
        return M_role_names;
    }

    /*!
      \brief get the position pair array
      \return const refrence to the array instance
     */
    const std::array< int, 11 > & positionPairs() const
    {
        return M_position_pairs;
    }

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
    const Data * data( const size_t idx ) const;

    /*!
      \brief get the data index nearest to the input point.
      \param pos input point.
      \param dist_thr distance threshold
      \return index of the result data. -1 if no result
    */
    int nearestDataIndex( const Vector2D & pos,
                          const double & dist_thr ) const;

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
      \brief set the role name
      \param unum target player
      \param name name value
      \return true if success
     */
    bool setRoleName( const int unum,
                      const std::string & name );

    /*!
      \brief set the position pair
      \param unum target player's uniform number
      \param paired_unum pared player's uniform number
      \return true if success
     */
    bool setPositionPair( const int unum,
                          const int paired_unum );

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

public:

    /*!
      \brief add new constraint between given indices.
      \param origin_idx index of first vertex.
      \param terminal_idx index of second vertex.
      \return error message if error occurd. otherwise, empty string.
    */
    std::string addConstraint( const size_t origin_idx,
                               const size_t terminal_idx );

    /*!
      \brief delete the specified constraint.
      \param idx replaced index.
      \param origin_idx new origin index.
      \param terminal_idx new terminal index.
      \return error message if error occurd. otherwise, empty string.
    */
    std::string replaceConstraint( const size_t idx,
                                   const size_t origin_idx,
                                   const size_t terminal_idx );

    /*!
      \brief delete the specified constraint.
      \param idx index of removed constraint.
      \return error message if error occurd. otherwise, empty string.
    */
    std::string removeConstraint( const size_t idx );

    /*!
      \brief delete the specified constraint.
      \param origin_idx origin index of removed constraint.
      \param terminal_idx terminal index of removed constraint.
      \return error message if error occurd. otherwise, empty string.
    */
    std::string removeConstraint( const size_t origin_idx,
                                  const size_t terminal_idx );


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
    bool readOld( std::istream & is );
    bool readCSV( std::istream & is );

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
    std::ostream & printOld( std::ostream & os ) const;
    std::ostream & printCSV( std::ostream & os ) const;
private:

    bool readV1( std::istream & is );

    bool readV2( std::istream & is,
                 const int data_size );

    bool readSample( std::istream & is,
                     const int index );
    bool readConstraints( std::istream & is );


    std::ostream & printV1( std::ostream & os ) const;
    std::ostream & printV2( std::ostream & os ) const;

    std::ostream & printConstraints( std::ostream & os ) const;

};
}

#endif
