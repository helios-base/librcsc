// -*-c++-*-

/*!
  \file object_table.h
  \brief seen object distance error table Header File
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

#ifndef RCSC_PLAYER_OBJECT_TABLE_H
#define RCSC_PLAYER_OBJECT_TABLE_H

#include <rcsc/player/view_mode.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/types.h>

#include <unordered_map>
#include <vector>

namespace rcsc {

/*!
  \class ObjectTable
  \brief predefined position map and distance table for observable objects
*/
class ObjectTable {
public:

    //! server epsilon parameter
    static const double SERVER_EPS;

    /*!
      \brief distance table entry
    */
    struct DataEntry {
        double M_seen_dist; //!< quantized distance
        double M_average; //!< predefined average distance
        double M_error; //!< predefined error range of quantized distance

        /*!
          \brief init variables by 0
        */
        DataEntry()
            : M_seen_dist( 0.0 )
            , M_average( 0.0 )
            , M_error( 0.0 )
          { }

        /*!
          \brief create data for search algorithm
          \param dist distance value to be searched
        */
        explicit
        DataEntry( const double & dist )
            : M_seen_dist( dist )
            , M_average(0.0)
            , M_error(0.0)
          { }

        /*!
          \brief create data with all parameters
          \param dist quantized distance
          \param ave predefined average distance
          \param err predefined error range
        */
        DataEntry( const double & dist,
                   const double & ave,
                   const double & err )
            : M_seen_dist( dist )
            , M_average( ave )
            , M_error( err )
          { }
    };

    //! type of marker map container
    typedef std::unordered_map< MarkerID, Vector2D > MarkerMap;

private:

    //! landmark map. key: marker id, value: coordinate value
    MarkerMap M_landmark_map;

    //! distance table for stationary objects (line, marker)
    std::vector< DataEntry > M_static_table;

    std::vector< DataEntry > M_static_table_v18_narrow; //!< landmark distance table for v18+ narrow
    std::vector< DataEntry > M_static_table_v18_normal; //!< landmark distance table for v18+ normal
    std::vector< DataEntry > M_static_table_v18_wide; //!< landmark distance table for v18+ wide

    //! distance table for movable objects (ball, player)
    std::vector< DataEntry > M_movable_table;

    std::vector< DataEntry > M_movable_table_v18_narrow; //!< distance table for v18+ narrow
    std::vector< DataEntry > M_movable_table_v18_normal; //!< distance table for v18+ normal
    std::vector< DataEntry > M_movable_table_v18_wide; //!< distance table for v18+ wide

public:
    /*!
      \brief create distance table
    */
    ObjectTable();


    /*!
      \brief get landmark map object
      \return const reference to the map container
    */
    const
    std::unordered_map< MarkerID, Vector2D > & landmarkMap() const
      {
          return M_landmark_map;
      }

    /*!
      \brief get predefined distance info for the stationary object
      \param see_dist seen distance
      \param ave variable pointer to store the result average distance
      \param err variable pointer to store the result error range
      \return true if found matched data entry
    */
    bool getStaticObjInfo( const double & see_dist,
                           double * ave,
                           double * err ) const;

    /*!
      \brief get predefined distance info for the movable object
      \param see_dist seen distance
      \param ave variable pointer to store the result average distance
      \param err variable pointer to store the result error range
      \return true if found matched data entry
    */
    bool getMovableObjInfo( const double & see_dist,
                            double * ave,
                            double * err ) const;

    /*!
      \brief get the predefinede distance range for the landmark objects (v18+)
      \param quant_dist seen distance value
      \param mean_dist variable pointer to store the result mean distance
      \param dist_error variable pointer to store the result error range
      \return true if found the matched data
     */
    bool getLandmarkDistanceRangeV18( const ViewWidth::Type view_width,
                                      const double quant_dist,
                                      double * mean_dist,
                                      double * dist_error ) const;

    bool getLandmarkDistanceRange( const double /*client_version*/,
                                   const ViewWidth::Type /*view_width*/,
                                   const double quant_dist,
                                   double * mean_dist,
                                   double * dist_error ) const
      {
          // return ( client_version >= 18.0
          //          ? getLandmarkDistanceRangeV18( view_width, quant_dist, mean_dist, dist_error )
          //          : getStaticObjInfo( quant_dist, mean_dist, dist_error ) );
          return getStaticObjInfo( quant_dist, mean_dist, dist_error );
      }

    /*!
      \brief get the predefinede distance range for the movable objects (v18+)
      \param quant_dist seen distance value
      \param mean_dist variable pointer to store the result mean distance
      \param dist_error variable pointer to store the result error range
      \return true if found the matched data
     */
    bool getDistanceRangeV18( const ViewWidth::Type view_width,
                              const double quant_dist,
                              double * mean_dist,
                              double * dist_error ) const;

    bool getDistanceRange( const double /*client_version*/,
                           const ViewWidth::Type /*view_width*/,
                           const double quant_dist,
                           double * mean_dist,
                           double * dist_error ) const
      {
          // return ( client_version >= 18.0
          //          ? getDistanceRangeV18( view_width, quant_dist, mean_dist, dist_error )
          //          : getMovableObjInfo( quant_dist, mean_dist, dist_error ) );
          return getMovableObjInfo( quant_dist, mean_dist, dist_error );
      }

    /*!
      \brief static utility. round real value
      \param value value to be rounded
      \param qstep precision step parameter
      \return rounded value
    */
    static
    double quantize( const double & value,
                     const double & qstep );

    /*!
      \brief static utility. quantize distance value by server's quantization method
      \param unq_dist distance value to be quantized
      \param qstep quantization step parameter
      \return quantized distance
    */
    static
    double quantize_dist( const double & unq_dist,
                          const double & qstep );

private:

    /*!
      \brief create landmark map
    */
    void createLandmarkMap();

    /*!
      \brief create distance table by built-in values
    */
    void createTable();

    /*!
      \brief create distance table dynamically
      \param static_qstap quantizaztion step for the stationary object
      \param movable_qstap quantizaztion step for the movable object

      It is not recommended to use this method for the real game
    */
    void create( const double & static_qstep,
                 const double & movable_qstep );

    /*!
      \brief implementation of distance table creation
      \param qstep quantizatin step
      \param container to be store the generated entries
    */
    void createTable( const double & qstep,
                      std::vector< DataEntry > & table );

};

}

#endif
