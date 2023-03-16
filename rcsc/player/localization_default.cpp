// -*-c++-*-

/*!
  \file localization_default.cpp
  \brief localization module Source File
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

#include "localization_default.h"

#include "object_table.h"
#include "body_sensor.h"
#include "world_model.h"

#include <rcsc/common/server_param.h>
#include <rcsc/common/logger.h>
#include <rcsc/geom/sector_2d.h>
#include <rcsc/time/timer.h>
#include <rcsc/math_util.h>

#include <algorithm>
#include <random>

using std::min;
using std::max;

// #define DEBUG_PROFILE
// #define DEBUG_PROFILE_REMOVE
// #define DEBUG_PRINT
// #define DEBUG_PRINT_SHAPE

#define USE_OBJECT_TABLE

namespace {
static int g_filter_count = 0;
}

namespace rcsc {

/*!
  \struct LocalizeImpl
  \brief localization implementation
*/
class LocalizationDefault::Impl {

private:

    //! object distance table
    ObjectTable M_object_table;

    //! grid point container
    std::vector< Vector2D > M_points;

public:
    /*!
      \brief create landmark map and object table
    */
    Impl()
        : M_object_table()
      {
          M_points.reserve( 1024 );
      }

    /*!
      \brief get object table
      \return const reference to the object table instance
    */
    const
    ObjectTable & objectTable() const
      {
          return M_object_table;
      }

    /*!
      \brief get grid points
      \return grid points
    */
    const
    std::vector< Vector2D > & points() const
      {
          return M_points;
      }

    //
    // self localization
    //

    /*!
      \brief update points using seen markers
      \param wm world model
      \param markers seen marker container
      \param self_face agent's global face angle
      \param self_face_err agent's global face angle error
    */
    void updatePointsByMarkers( const WorldModel & wm,
                                const VisualSensor::MarkerCont & markers,
                                const double & self_face,
                                const double & self_face_err );

    /*!
      \brief update points using seen markers
      \param wm world model
      \param markers seen marker container
      \param behind_markers behind marker container
      \param self_pos agent's global position
      \param self_face agent's global face angle
      \param self_face_err agent's global face angle error
    */
    void updatePointsByBehindMarker( const WorldModel & wm,
                                     const VisualSensor::MarkerCont & markers,
                                     const VisualSensor::MarkerCont & behind_markers,
                                     const Vector2D & self_pos,
                                     const double & self_face,
                                     const double & self_face_err,
                                     const GameTime & current );

    /*!
      \brief update points by one marker
      \param wm world model
      \param marker seen marker info
      \param id estimated marker's Id
      \param self_face agent's global face angle
      \param self_face_err agent's global face angle error
    */
    void updatePointsBy( const WorldModel & wm,
                         const VisualSensor::MarkerT & marker,
                         const MarkerID id,
                         const double & self_face,
                         const double & self_face_err );

    /*!
      \brief calculate average point and error with all points.
      \param ave_pos pointer to the variable to store the averaged point
      \param ave_err pointer to the variable to store the averaged point error
    */
    void averagePoints( Vector2D * ave_pos,
                        Vector2D * ave_err );

    /*!
      \brief generate candidate points using nearest marker
      \param wm world model
      \param marker seen marker object
      \param self_face agent's global face angle
      \param self_face_err agent's global face angle error
    */
    void generatePoints( const WorldModel & wm,
                         const VisualSensor::MarkerT & marker,
                         const MarkerID id,
                         const double & self_face,
                         const double & self_face_err );

    void resamplePoints( const WorldModel & wm,
                         const VisualSensor::MarkerT & marker,
                         const MarkerID id,
                         const double & self_face,
                         const double & self_face_err );

    //
    // utility
    //

    /*!
      \brief get nearest marker flag Id. include goal check
      \param objtype used only to detect 'goal' or 'flag'
      \param pos estimated position
      \return marker ID

      This method is used to identify the behind marker object.
    */
    MarkerID getNearestMarker( const VisualSensor::ObjectType objtype,
                               const Vector2D & pos ) const;

    /*!
      \brief get global angle & angle range
      \param seen_dir seen dir
      \param self_face agent's global face angle
      \param self_face_err agent's global face angle error
      \param average pointer to the variable to store the averaged angle
      \param err pointer to the variable to store the estimated angle error.
    */
    void getDirRange( const double & seen_dir,
                      const double & self_face,
                      const double & self_face_err,
                      double * average,
                      double * err );

    /*!
      \brief calculate the unquantized distance information using the inverse algorithm
      \param client_version client protocol version
      \param view_width client's view width
      \param quant_dist the quantized distance information sent by the server
      \param qstep the quantize step parameter
      \param mean_dist the result value of the mean distance
      \param dist_error the result error value
     */
    void inverseDistanceRange( const double client_version,
                               const ViewWidth::Type view_width,
                               const double quant_dist,
                               const double qstep,
                               double * mean_dist,
                               double * dist_error ) const;
    // get unquantized dist range
    // void getDistRange(const double &see_dist, const double &qstep,
    //                   double *average, double *range);


    /*!
      \brief estimate self global face angle from seen markers
      \param markers seen marker container
      \return self face angle. if failed, retun VisualSensor::DIR_ERR
    */
    double getFaceDirByMarkers( const WorldModel & wm,
                                const VisualSensor::MarkerCont & markers ) const;

    /*!
      \brief estimate self global face angle from seen lines
      \param lines seen line info
      \return self face angle. if failed, retun VisualSensor::DIR_ERR
    */
    double getFaceDirByLines( const VisualSensor::LineCont & lines ) const;
};

/*-------------------------------------------------------------------*/
/*!

 */
MarkerID
LocalizationDefault::Impl::getNearestMarker( const VisualSensor::ObjectType objtype,
                                             const Vector2D & pos ) const
{
    // check closest behind goal
    if ( objtype == VisualSensor::Obj_Goal_Behind )
    {
        return ( pos.x < 0.0 ? Goal_L : Goal_R );
    }

    // check nearest behind flag

    // Magic Number (related visible_distance and marker's space)
    //double mindist2 = 2.4 * 2.4
    double mindist2 = 3.0 * 3.0;
    MarkerID candidate = Marker_Unknown;

    for ( const auto & v : objectTable().landmarkMap() )
    {
        double d2 = pos.dist2( v.second );
        if ( d2 < mindist2 )
        {
            mindist2 = d2;
            candidate = v.first;
        }
    }

#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  "localizer.getClosesetMarker. candidate = %d",
                  candidate );
#endif
    return candidate;
}

/*-------------------------------------------------------------------*/
/*!

 */
double
LocalizationDefault::Impl::getFaceDirByLines( const VisualSensor::LineCont & lines ) const
{
    if ( lines.empty() )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      __FILE__" (getFaceDirFromLines) no lines!!" );
#endif
        return VisualSensor::DIR_ERR;
    }

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //  lines must be sorted by distance from self.
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    double angle = lines.front().dir_;

#ifdef OLD_DIR_ROUND
    if ( angle > 0.0 ) angle += 0.5;
    if ( angle < 0.0 ) angle -= 0.5;
#endif

    if ( angle < 0.0 )
    {
        angle += 90.0;
    }
    else
    {
        angle -= 90.0;
    }

    switch ( lines.front().id_ ) {
    case Line_Left:
        angle = 180.0 - angle;
        break;
    case Line_Right:
        angle = 0.0 - angle;
        break;
    case Line_Top:
        angle = -90.0 - angle;
        break;
    case Line_Bottom:
        angle = 90.0 - angle;
        break;
    default:
        std::cerr << __FILE__ << ": " << __LINE__
                  << " Invalid line type " << lines.front().id_
                  << std::endl;
        return angle;
    }

    // out of field
    if ( lines.size() >= 2 )
    {
        angle += 180.0;
    }

    angle = AngleDeg::normalize_angle( angle );

    return angle;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
LocalizationDefault::Impl::updatePointsByMarkers( const WorldModel & wm,
                                                  const VisualSensor::MarkerCont & markers,
                                                  const double & self_face,
                                                  const double & self_face_err )
{
    // must check marker container is NOT empty.

    const VisualSensor::MarkerCont::const_iterator end = markers.end();
    VisualSensor::MarkerCont::const_iterator marker = markers.begin();

    // start from second nearest marker,
    // because first marker is used for the initial point set generation
    ++marker;

    int count = 0;
    g_filter_count = 0;
    for ( ;
          marker != end && count < 30; // magic number
          ++marker, ++count )
    {
        ++g_filter_count;
        updatePointsBy( wm, *marker, marker->id_, self_face, self_face_err );
        resamplePoints( wm, markers.front(), markers.front().id_, self_face, self_face_err );
    }

#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  __FILE__" (updatePointsByMarkers) filtered marker count = %d", count );
#endif
}

/*-------------------------------------------------------------------*/
/*!

 */
void
LocalizationDefault::Impl::updatePointsByBehindMarker( const WorldModel & wm,
                                                       const VisualSensor::MarkerCont & markers,
                                                       const VisualSensor::MarkerCont & behind_markers,
                                                       const Vector2D & self_pos,
                                                       const double & self_face,
                                                       const double & self_face_err,
#ifdef DEBUG_PRINT
                                                       const GameTime & current
#else
                                                       const GameTime &
#endif
                                                       )
{
    ////////////////////////////////////////////////////////////////////
    // estimate mypos using CLOSE behind markers
    if ( behind_markers.empty() )
    {
        // nothing to do
        return;
    }

    // matching behind marker
    MarkerID marker_id = getNearestMarker( behind_markers.front().object_type_, self_pos );

    if ( marker_id == Marker_Unknown )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      __FILE__" (updatePointsByBehindMarker) "
                      " failed to find  BEHIND marker Id" );
#endif
        return;
    }

    ////////////////////////////////////////////////////////////////////
    // update points using closest behind marker's sector
#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  __FILE__" (updatePointsByBehindMarker)"
                  " update by BEHIND marker" );
#endif

    updatePointsBy( wm,
                    behind_markers.front(),
                    marker_id,
                    self_face, self_face_err );

    if ( points().empty() )
    {
#ifdef DEBUG_PRINT
        std::cerr << __FILE__ << ": " << current
                  << " re-generate points by behind marker."
                  << std::endl;
        dlog.addText(  Logger::WORLD,
                       __FILE__" (updatePointsByBehindMarker) re-generate points." );
#endif

        generatePoints( wm, behind_markers.front(), marker_id, self_face, self_face_err );

        if ( points().empty() )
        {
#ifdef DEBUG_PRINT
            std::cerr << __FILE__ << ": no candidate point by behind marker!!" << std::endl;
            dlog.addText( Logger::WORLD,
                          __FILE__" (updatePointsByBehindMarker) no points by behind marker." );
#endif
            return;
        }

        g_filter_count = 0;

        int count = 0;
        for ( VisualSensor::MarkerCont::const_iterator marker = markers.begin(), end = markers.end();
              marker != end && count < 20;
              ++marker, ++count )
        {
            ++g_filter_count;
            updatePointsBy( wm, *marker, marker->id_, self_face, self_face_err );
            resamplePoints( wm, markers.front(), markers.front().id_, self_face, self_face_err );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
LocalizationDefault::Impl::updatePointsBy( const WorldModel & wm,
                                           const VisualSensor::MarkerT & marker,
                                           const MarkerID id,
                                           const double & self_face,
                                           const double & self_face_err )
{
    ////////////////////////////////////////////////////////////////////
    // get marker global position
    ObjectTable::MarkerMap::const_iterator it = objectTable().landmarkMap().find( id );
    if ( it == objectTable().landmarkMap().end() )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " why cannot find nearest behind marker id ??"
                  << std::endl;
        dlog.addText( Logger::WORLD,
                      __FILE__" (updatePointsBy)"
                      " why cannot find CLOSE behind marker id ??" );
        return;
    }

    const Vector2D & marker_pos = it->second;

    ////////////////////////////////////////////////////////////////////
    // get polar range info
    double ave_dist, dist_error;

    // get distance range info
#ifdef USE_OBJECT_TABLE
    if ( ! objectTable().getLandmarkDistanceRange( wm.clientVersion(),
                                                   wm.self().viewWidth().type(),
                                                   marker.dist_, &ave_dist, &dist_error ) )
    {
        std::cerr << __FILE__ << " (updatePointsBy) unexpected marker distance "
                  << marker.dist_ << std::endl;
        dlog.addText( Logger::WORLD,
                      __FILE__" (updatePointsBy) unexpected marker distance = %f",
                      marker.dist_ );
        return;
    }
#else
    inverseDistanceRange( wm.clientVersion(), wm.self().viewWidth().type(),
                          marker.dist_, ServerParam::i().landmarkDistQuantizeStep(), &ave_dist, &dist_error );
#endif

    // get dir range info
    double ave_dir, dir_error;
    getDirRange( marker.dir_, self_face, self_face_err, &ave_dir, &dir_error );
    // reverse, because base point calculated in above function is marker point.
    ave_dir += 180.0;

    ////////////////////////////////////////////////////////////////////
    // create candidate sector
    const Sector2D sector( marker_pos, // base point
                           ave_dist - dist_error, // min dist
                           ave_dist + dist_error, // max dist
                           AngleDeg( ave_dir - dir_error ), // start left angle
                           AngleDeg( ave_dir + dir_error ) ); // end right angle

#if 0
    {
        // display candidate area
        Vector2D v1 = Vector2D::polar2vector( sector.radiusMax(), sector.angleLeftStart() );
        Vector2D v2 = Vector2D::polar2vector( sector.radiusMax(), sector.angleRightEnd() );
        Vector2D v3 = Vector2D::polar2vector( sector.radiusMin(), sector.angleLeftStart() );
        Vector2D v4 = Vector2D::polar2vector( sector.radiusMin(), sector.angleRightEnd() );
        v1 += marker_pos;
        v2 += marker_pos;
        v3 += marker_pos;
        v4 += marker_pos;

        int r = 16 * ( g_filter_count % 16 );
        int g = 16 * ( ( g_filter_count + 5 ) % 16 );
        int b = 16 * ( ( g_filter_count + 10 ) % 16 );
        char col[8];
        snprintf( col, 8, "#%02x%02x%02x", r, g, b );
        dlog.addLine( Logger::WORLD, v1, v2, col );
        dlog.addLine( Logger::WORLD, v2, v4, col );
        dlog.addLine( Logger::WORLD, v4, v3, col );
        dlog.addLine( Logger::WORLD, v3, v1, col );
    }
#endif
#ifdef DEBUG_PRINT_SHAPE
    {

        int r = 16 * ( g_filter_count % 16 );
        int g = 16 * ( ( g_filter_count + 5 ) % 16 );
        int b = 16 * ( ( g_filter_count + 10 ) % 16 );
        char col[8];
        snprintf( col, 8, "#%02x%02x%02x", r, g, b );

        dlog.addSector( Logger::WORLD,
                        sector, col );
    }
#endif

    // check whether points are within candidate sector
    // not contained points are erased from container.

#ifdef DEBUG_PROFILE_REMOVE
    Timer timer;
    int initial_size = M_points.size();
#endif

    M_points.erase( std::remove_if( M_points.begin(),
                                    M_points.end(),
                                    [&]( const Vector2D & p )
                                      {
                                          return ! sector.contains( p );
                                      } ),
                    M_points.end() );

#ifdef DEBUG_PROFILE_REMOVE
    dlog.addText( Logger::WORLD,
                  __FILE__" (updatePointsBy) elapsed %f [ms] points=%d -> %d",
                  timer.elapsedReal(),
                  initial_size, (int)points().size() );
#endif

#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  __FILE__" (updatePointsBy) points=%d marker(% 7.2f, % 7.2f)"
                  " dist=%f, dist_range=%f"
                  " dir=%.1f, dir_range=%.1f",
                  (int)M_points.size(),
                  marker_pos.x, marker_pos.y,
                  ave_dist, dist_error * 2.0,
                  ave_dir, dir_error * 2.0 );
#endif
}

/*-------------------------------------------------------------------*/
/*!

 */
void
LocalizationDefault::Impl::averagePoints( Vector2D * ave_pos,
                                          Vector2D * ave_err )
{
    ave_pos->assign( 0.0, 0.0 );
    ave_err->assign( 0.0, 0.0 );

    if ( M_points.empty() )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      __FILE__" (averagePoints) Empty!." );
#endif
        return;
    }

#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  __FILE__" (averagePoints) rest %d points.",
                  M_points.size() );
#endif

    double max_x, min_x, max_y, min_y;

    max_x = min_x = M_points.front().x;
    max_y = min_y = M_points.front().y;

    for ( const Vector2D & p : M_points )
    {
        *ave_pos += p;
#ifdef DEBUG_PRINT_SHAPE
        // display points
        dlog.addCircle( Logger::WORLD,
                        p, 0.005,
                        "#ff0000",
                        true ); // fill
#endif
        if ( p.x > max_x )
        {
            max_x = p.x;
        }
        else if ( p.x < min_x )
        {
            min_x = p.x;
        }

        if ( p.y > max_y )
        {
            max_y = p.y;
        }
        else if ( p.y < min_y )
        {
            min_y = p.y;
        }
    }

    *ave_pos /= static_cast< double >( M_points.size() );

#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  __FILE__" (averagePoints) self_pos=(%.3f, %.3f)"
                  "  err_x_range=(%.3f, %.3f)  err_y_range(%.3f, %.3f)",
                  ave_pos->x, ave_pos->y,
                  min_x, max_x, min_y, max_y );
#endif
#ifdef DEBUG_PRINT_SHAPE
    dlog.addCircle( Logger::WORLD,
                    *ave_pos, 0.01,
                    "#0000ff",
                    true ); // fill
#endif

    ave_err->x = ( max_x - min_x ) * 0.5;
    ave_err->y = ( max_y - min_y ) * 0.5;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
LocalizationDefault::Impl::generatePoints( const WorldModel & wm,
                                           const VisualSensor::MarkerT & marker,
                                           const MarkerID id,
                                           const double & self_face,
                                           const double & self_face_err )

{
    // marker must be the nearest one.

    ////////////////////////////////////////////////////////////////////
    // clear old points
    M_points.clear();


    ////////////////////////////////////////////////////////////////////
    // get closest marker info

    ObjectTable::MarkerMap::const_iterator marker_it = objectTable().landmarkMap().find( id );
    if ( marker_it == objectTable().landmarkMap().end() )
    {
        std::cerr << __FILE__ << " (generatePoints) cannot find marker id ??"
                  << std::endl;
        return;
    }

    const Vector2D marker_pos = marker_it->second;

    ////////////////////////////////////////////////////////////////////
    // get sector range

    double ave_dist, dist_error;
#ifdef USE_OBJECT_TABLE
    if ( ! objectTable().getLandmarkDistanceRange( wm.clientVersion(),
                                                   wm.self().viewWidth().type(),
                                                   marker.dist_,  &ave_dist, &dist_error ) )
    {
        std::cerr << __FILE__ << " (generatePoints) marker dist error" << std::endl;
        return;
    }
#else
    inverseDistanceRange( wm.clientVersion(), wm.self().viewWidth().type(),
                          marker.dist_, ServerParam::i().landmarkDistQuantizeStep(), &ave_dist, &dist_error );
#endif

    double ave_dir, dir_error;
    getDirRange( marker.dir_,
                 self_face, self_face_err,
                 &ave_dir, &dir_error );

    // reverse dir, because base point is marker point
    ave_dir += 180.0;

    const double min_dist = ave_dist - dist_error;
    const double dist_range = dist_error * 2.0;
    double dist_inc = std::max( 0.01, dist_error / 16.0 );
    const int dist_loop = bound( 2,
                                 static_cast< int >( std::ceil( dist_range / dist_inc ) ),
                                 16 );
    dist_inc = dist_range / ( dist_loop - 1 );

    const double dir_range = dir_error * 2.0;
    const double circum = 2.0 * ave_dist * M_PI * ( dir_range / 360.0 );
    double circum_inc = std::max( 0.01, circum / 32.0 );
    const int dir_loop = bound( 2,
                                static_cast< int >( std::ceil( circum / circum_inc ) ),
                                32 );
    const double dir_inc = dir_range / ( dir_loop - 1 );

    AngleDeg base_angle( ave_dir - dir_error ); // left first;
    for ( int idir = 0; idir < dir_loop; ++idir, base_angle += dir_inc )
    {
        Vector2D base_vec = Vector2D::polar2vector( 1.0, base_angle );

        double add_dist = 0.0;
        for ( int idist = 0; idist < dist_loop; ++idist, add_dist += dist_inc )
        {
            M_points.push_back( marker_pos
                                + ( base_vec * ( min_dist + add_dist ) ) );
#ifdef DEBUG_PRINT_SHAPE
            dlog.addCircle( Logger::WORLD,
                            M_points.back(), 0.01,
                            "#ffff00" );
#endif
        }
    }

#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  __FILE__" (generatePoints) generate %d points by marker(%.1f %.1f)",
                  (int)M_points.size(), marker_pos.x, marker_pos.y );
    dlog.addText( Logger::WORLD,
                  __FILE__" _____  dir_loop=%d dir_inc=%.3f dir_range=%.3f",
                  dir_loop, dir_inc, dir_range );
    dlog.addText( Logger::WORLD,
                  __FILE__" _____  dist_loop=%d dist_inc=%.3f dist_range=%.3f",
                  dist_loop, dist_inc, dist_range );
#endif


#if 0
    dlog.addText( Logger::WORLD,
                  __FILE__" (generatePoints) base_marker=(%f, %f)  dist=%f  range=%f"
                  "  dir=%f  range=%f",
                  marker_pos.x, marker_pos.y, ave_dist, dist_range,
                  ave_dir, dir_range );
    dlog.addText( Logger::WORLD,
                  __FILE__" (generatePoints) first point (%f, %f)",
                  M_points.front().x, M_points.front().y );
#endif
#if 0
    // display candidate area
    Vector2D v1 = Vector2D::polar2vector( min_dist + dist_range, AngleDeg( ave_dir - dir_error ) );
    Vector2D v2 = Vector2D::polar2vector( min_dist + dist_range, AngleDeg( ave_dir + dir_error ) );
    Vector2D v3 = Vector2D::polar2vector( min_dist, AngleDeg( ave_dir - dir_error ) );
    Vector2D v4 = Vector2D::polar2vector( min_dist, AngleDeg( ave_dir + dir_error ) );
    v1 += marker_pos;
    v2 += marker_pos;
    v3 += marker_pos;
    v4 += marker_pos;

    dlog.addLine( Logger::WORLD, v1, v2, "#ffffff" );
    dlog.addLine( Logger::WORLD, v2, v4, "#ffffff" );
    dlog.addLine( Logger::WORLD, v4, v3, "#ffffff" );
    dlog.addLine( Logger::WORLD, v3, v1, "#ffffff" );
#endif
#ifdef DEBUG_PRINT_SHAPE
    dlog.addSector( Logger::WORLD,
                    marker_pos,
                    min_dist, min_dist + dist_range,
                    AngleDeg( ave_dir - dir_error ), dir_range,
                    "#000000" );
#endif
}

/*-------------------------------------------------------------------*/
/*!

 */
void
LocalizationDefault::Impl::resamplePoints( const WorldModel & wm,
                                           const VisualSensor::MarkerT & marker,
                                           const MarkerID id,
                                           const double & self_face,
                                           const double & self_face_err )
{
    static std::mt19937 s_engine( 49827140 );
    static const size_t max_count = 50;

    const std::size_t count = M_points.size();

    if ( count >= max_count )
    {
        return;
    }

    if ( count == 0 )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      __FILE__" (resamplePoints) no points. regenerate..." );
#endif
        generatePoints( wm, marker, id, self_face, self_face_err );
        return;
    }

    // generate additional points using valid points coordinate
    // x & y are generated independently.
    // result may not be within current candidate sector

    std::uniform_real_distribution<> xy_dst( -0.01, 0.01 );

#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  __FILE__" (resamplePoints) generate %d points",
                  (int)(max_count - count) );
#endif

    std::uniform_int_distribution<> index_dst( 0, count - 1 );

    for ( size_t i = count; i < max_count; ++i )
    {
        M_points.push_back( M_points[index_dst( s_engine )]
                            + Vector2D( xy_dst( s_engine ), xy_dst( s_engine ) ) );
#ifdef DEBUG_PRINT_SHAPE
        dlog.addCircle( Logger::WORLD,
                        M_points.back(), 0.01,
                        "#ff0000" );
#endif
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
LocalizationDefault::Impl::getDirRange( const double & seen_dir,
                                        const double & self_face,
                                        const double & self_face_err,
                                        double * average,
                                        double * err )
{
#ifdef OLD_DIR_ROUND

    if ( seen_dir == 0.0 ) *average = 0.0;
    if ( seen_dir > 0.0 )  *average = AngleDeg::normalize_angle(seen_dir + 0.5);
    if ( seen_dir < 0.0 )  *average = AngleDeg::normalize_angle(seen_dir - 0.5);

    *err = 0.5;
    if ( seen_dir == 0.0 ) *err = 1.0;

#else

    *average = seen_dir;
    *err = 0.5;

#endif

    *average += self_face;
    *err += self_face_err;
}

/*-------------------------------------------------------------------*/
/*!

 */
double
LocalizationDefault::Impl::getFaceDirByMarkers( const WorldModel & wm,
                                                const VisualSensor::MarkerCont & markers ) const
{
    double angle = VisualSensor::DIR_ERR;

    // get my face from two seen markers
    if ( markers.size() < 2 )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      __FILE__" (getFaceDirByMarkers) marker size less than 2."
                      " cannot get self face" );
#endif
        return angle;
    }

#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  __FILE__" (getFaceDirByMarkers) try to get face from 2 markers" );
#endif

    ObjectTable::MarkerMap::const_iterator it1 = objectTable().landmarkMap().find( markers.front().id_ );
    if ( it1 == objectTable().landmarkMap().end() )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      __FILE__" (getFaceDirByMarkers) cannot get marker1" );
#endif
        return angle;
    }

    ObjectTable::MarkerMap::const_iterator it2 = objectTable().landmarkMap().find( markers.back().id_ );
    if ( it2 == objectTable().landmarkMap().end() )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      __FILE__" (getFaceDirByMarkers) cannot get marker2" );
#endif
        return angle;
    }

    double marker_dist1, marker_dist2, tmperr;
#ifdef USE_OBJECT_TABLE
    if ( ! objectTable().getLandmarkDistanceRange( wm.clientVersion(),
                                                   wm.self().viewWidth().type(),
                                                   markers.front().dist_,
                                                   &marker_dist1,
                                                   &tmperr ) )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      __FILE__" (getFaceDirByMarkers) cannot get face(3)" );
#endif
        return angle;
    }
    if ( ! objectTable().getLandmarkDistanceRange( wm.clientVersion(),
                                                   wm.self().viewWidth().type(),
                                                   markers.back().dist_,
                                                   &marker_dist2,
                                                   &tmperr ) )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      __FILE__" (getFaceDirByMarkers) cannot get face(4)" );
#endif
        return angle;
    }
#else
    inverseDistanceRange( wm.clientVersion(), wm.self().viewWidth().type(),
                          markers.front().dist_, ServerParam::i().landmarkDistQuantizeStep(), &marker_dist1, &tmperr );
    inverseDistanceRange( wm.clientVersion(), wm.self().viewWidth().type(),
                          markers.back().dist_, ServerParam::i().landmarkDistQuantizeStep(), &marker_dist2, &tmperr );
#endif

    Vector2D rpos1 = Vector2D::polar2vector( marker_dist1, markers.front().dir_ );
    Vector2D rpos2 = Vector2D::polar2vector( marker_dist2, markers.back().dir_ );
    Vector2D gap1 = rpos1 - rpos2;
    Vector2D gap2 = it1->second - it2->second;

    angle = ( gap2.th() - gap1.th() ).degree();

#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  __FILE__" (getFaceDirByMarkers) get face from 2 flags. angle = %f",
                  angle );
#endif
    return angle;
}

/*-------------------------------------------------------------------*/
void
LocalizationDefault::Impl::inverseDistanceRange( const double /*client_version*/,
                                                 const ViewWidth::Type /*view_width*/,
                                                 const double quant_dist,
                                                 const double qstep_base,
                                                 double * mean_dist,
                                                 double * dist_error ) const
{
   /*
     === server quantize algorithm ===

      d1 = log( unq_dist + EPS )
      d2 = rint( d1 / qstep ) * qstep // quantize( d1, qstep )
      d3 = exp( d2 )
      quant_dist = rint( d3 / 0.1 ) * 0.1 // quantize( d3, 0.1 )
    */

    /*
      === unquantize (inverse quantize) algorithm ===

      min_d3 = (rint(quant_dist / 0.1) - 0.5) * 0.1
      max_d3 = (rint(quant_dist / 0.1) + 0.5) * 0.1

      min_d2 = log( min_d3 )
      max_d2 = log( max_d3 )

      min_d1 = (rint(min_d2 / qstep) - 0.5) * qstep
      max_d1 = (rint(min_d2 / qstep) + 0.5) * qstep

      min_d = exp( min_d1 ) - EPS
      max_d = exp( max_d1 ) - EPS

    */

    // const double qstep = ( client_version < 18.0
    //                        ? qstep_base
    //                        : view_width == ViewWidth::NARROW
    //                        ? qstep_base * 0.5
    //                        : view_width == ViewWidth::NORMAL
    //                        ? qstep_base * 0.75
    //                        : qstep_base * 1.0 );
    const double qstep = qstep_base;

    double min_dist, max_dist;

    if ( quant_dist < ObjectTable::SERVER_EPS )
    {
        min_dist = 0.0;
    }
    else
    {
        min_dist = ( rint( quant_dist / 0.1 ) - 0.5 ) * 0.1;
        min_dist = std::min( std::log( min_dist - 0.05 ), std::log( min_dist + 0.05 ) );
        min_dist = ( rint( min_dist / qstep ) - 0.5 ) * qstep;
        min_dist = std::exp( min_dist );
    }

    max_dist = ( rint( quant_dist / 0.1 ) + 0.5 ) * 0.1;
    max_dist = std::log( max_dist );
    max_dist = ( rint( max_dist / qstep ) + 0.5 ) * qstep;
    max_dist = std::exp( max_dist );

    *mean_dist = ( max_dist + min_dist ) * 0.5;
    *dist_error = ( max_dist - min_dist ) * 0.5;
}


#if 0
/*-------------------------------------------------------------------*/
/*
  get distance and distance range using rcssserver settings
*/
void
LocalizationDefault::Impl::getDistRange( const double & seen_dist,
                                         const double & qstep,
                                         double * average,
                                         double * range )
{
    /*
      server quantize algorithm

      d1 = log( unq_dist + EPS )

      d2 = quantize( d1 , qstep )

      d3 = exp( d2 )

      quant_dist = quantize( d3, 0.1 )
    */

    /*
      unquantize (inverse quantize) algorithm

      min_d3 = (rint(quant_dist / 0.1) - 0.5) * 0.1
      max_d3 = (rint(quant_dist / 0.1) + 0.5) * 0.1

      min_d2 = log( min_d3 )
      max_d2 = log( max_d3 )

      min_d1 = (rint(min_d2 / qstep) - 0.5) * qstep
      max_d1 = (rint(min_d2 / qstep) + 0.5) * qstep

      min_d = exp( min_d1 ) - EPS
      max_d = exp( max_d1 ) - EPS

    */


    // first rint is not needed ;)

    // first +-0.5 is ignored,
    // because important error is occured in close distance case.

    double min_dist = ( std::round( std::log( seen_dist ) / qstep ) - 0.5 ) * qstep;
    min_dist = std::exp( min_dist ) - ObjectTable::SERVER_EPS;

    double max_dist = ( std::round( std::log( seen_dist ) / qstep ) + 0.5 ) * qstep;
    max_dist = std::exp( max_dist ) - ObjectTable::SERVER_EPS;

    *range = max_dist - min_dist;
    if ( *range < ObjectTable::SERVER_EPS )
    {
        *range = 0.05;
    }
    else if ( *range < 0.1 )
    {
        *range = 0.1;
    }

    *average = ( max_dist + min_dist ) * 0.5;
    if ( *average < ObjectTable::SERVER_EPS )
    {
        *average = *range * 0.5;
    }

    /*
      double tmp;
      if (min_dist)
      {
      //tmp = (rint(seen_dist / 0.1) - 0.5) * 0.1;
      //tmp = (seen_dist / 0.1 - 0.5) * 0.1;
      tmp = seen_dist;// - 0.05;
      if (tmp <= 0.0) tmp = SERVER_EPS;
      //tmp = log(tmp);
      tmp = (rint(log(tmp) / qstep) - 0.5) * qstep;
      *min_dist = exp(tmp) - SERVER_EPS;
      }

      if (max_dist)
      {
      //tmp = (rint(seen_dist / 0.1) + 0.5) * 0.1;
      //tmp = (seen_dist / 0.1 + 0.5) * 0.1;
      tmp = seen_dist;// + 0.05;
      //tmp = log(tmp);
      tmp = (rint(log(tmp) / qstep) + 0.5) * qstep;
      *max_dist = exp(tmp) - SERVER_EPS;
      }

      *range = *max_dist - *min_dist;
      if (*range < SERVER_EPS) *range = 0.05;
      else if (*range < 0.1) *range = 0.1;

      *average = (*max_dist + *min_dist) * 0.5;
      if (*average < SERVER_EPS) *average = *range * 0.5;
    */
}

#endif

/////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

 */
LocalizationDefault::LocalizationDefault()
    : M_impl( new Impl() )
{

}

/*-------------------------------------------------------------------*/
/*!

 */
LocalizationDefault::~LocalizationDefault()
{

}

/*-------------------------------------------------------------------*/
/*!

 */
bool
LocalizationDefault::updateBySenseBody( const BodySensor & )
{
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
LocalizationDefault::estimateSelfFace( const WorldModel & wm,
                                       const VisualSensor & see,
                                       double * self_face,
                                       double * self_face_err )
{
    *self_face = M_impl->getFaceDirByLines( see.lines() );

    if ( *self_face == VisualSensor::DIR_ERR )
    {
        *self_face = M_impl->getFaceDirByMarkers( wm, see.markers() );
        if ( *self_face == VisualSensor::DIR_ERR )
        {
#ifdef DEBUG_PRINT
            dlog.addText( Logger::WORLD,
                          __FILE__" (estimateSelfFace) cannot get self face" );
#endif
            return false;
        }
    }

#ifdef OLD_DIR_ROUND
    *self_face_err = ( *self_face == 0.0 ) ? 1.0 : 0.5;
#else
    *self_face_err = 0.5;
#endif

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
LocalizationDefault::localizeSelf( const WorldModel & wm,
                                   const VisualSensor & see,
                                   const ActionEffector & /*act*/,
                                   const double self_face,
                                   const double self_face_err,
                                   Vector2D * self_pos,
                                   Vector2D * self_pos_err )
{
    // !!! NOTE !!!
    // markers must be sorted by distance from self

    // initialize
    // self_pos must be assigned ERROR_VALUE
    self_pos->invalidate();
    self_pos_err->assign( 0.0, 0.0 );

    ////////////////////////////////////////////////////////////////////
    // if no marker, we cannot estimate my position
    if ( see.markers().empty() )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      __FILE__" (localizeSelf) no marker!" );
#endif
        return false;
    }

#ifdef DEBUG_PROFILE
    Timer timer;
#endif

    ////////////////////////////////////////////////////////////////////
    // generate points using the nearest marker
    M_impl->generatePoints( wm,
                            see.markers().front(),
                            see.markers().front().id_,
                            self_face,
                            self_face_err );

    if ( M_impl->points().empty() )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      __FILE__" (localizeSelf) no points! (1)" );
#endif
        return false;
    }

#ifdef DEBUG_PROFILE
    int initial_point_size = M_impl->points().size();
    Timer update_timer;
#endif
    ////////////////////////////////////////////////////////////////////
    // update points by known markers
    M_impl->updatePointsByMarkers( wm,
                                   see.markers(),
                                   self_face,
                                   self_face_err );
#ifdef DEBUG_PROFILE
    double update_time = update_timer.elapsedReal();
#endif

    // in order to estimate the Id of nearest behind marker,
    // it is necessary to calculate current estimation result,
    M_impl->averagePoints( self_pos, self_pos_err );

    if ( ! see.behindMarkers().empty() )
    {
        // update points by nearest behind marker
        M_impl->updatePointsByBehindMarker( wm,
                                            see.markers(),
                                            see.behindMarkers(),
                                            *self_pos,
                                            self_face,
                                            self_face_err,
                                            see.time() );
        // re-calculate average pos
        M_impl->averagePoints( self_pos, self_pos_err );
    }

#ifdef DEBUG_PROFILE
    dlog.addText( Logger::WORLD,
                  __FILE__" (localizeSelf) elapsed %f (update=%f) [ms] marker= %d points= %d -> %d",
                  timer.elapsedReal(), update_time,
                  (int)see.markers().size(),
                  initial_point_size,
                  (int)M_impl->points().size() );
#endif

    return self_pos->isValid();
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
LocalizationDefault::localizeBallRelative( const WorldModel & wm,
                                           const VisualSensor & see,
                                           const double self_face,
                                           const double self_face_err,
                                           Vector2D * rpos,
                                           Vector2D * rpos_err,
                                           Vector2D * rvel,
                                           Vector2D * rvel_err ) const
{
    if ( see.balls().empty() )
    {
        return false;
    }

    const VisualSensor::BallT & ball = see.balls().front();

    ////////////////////////////////////////////////////////////////////
    // get polar range info
    double average_dist, dist_error;
#ifdef USE_OBJECT_TABLE
    // dist range
    if ( ! M_impl->objectTable().getDistanceRange( wm.clientVersion(),
                                                   wm.self().viewWidth().type(),
                                                   ball.dist_, &average_dist, &dist_error ) )
    {
        std::cerr << __FILE__ << " (localizeBallRelative) unexpected ball distance "
                  << ball.dist_ << std::endl;
        dlog.addText( Logger::WORLD,
                      __FILE__" (localizeBallRelative) unexpected ball distance %f",
                      ball.dist_ );
        return false;
    }
#else
    M_impl->inverseDistanceRange( wm.clientVersion(), wm.self().viewWidth().type(),
                                  ball.dist_, ServerParam::i().distQuantizeStep(), &average_dist, &dist_error );
#endif

    // dlog.addText( Logger::WORLD,
    //               __FILE__" (localizeBallRelative) self_face=%.1f err=%.3f",
    //               self_face, self_face_err );

    // dir range
    double average_dir, dir_error;
    M_impl->getDirRange( ball.dir_,
                         self_face, self_face_err,
                         &average_dir, &dir_error );

    const double max_dist = average_dist + dist_error;
    const double min_dist = average_dist - dist_error;
    const AngleDeg max_ang = average_dir + dir_error;
    const AngleDeg min_ang = average_dir - dir_error;

    /*
      TRACEWM(DEBUG_STRM << "Ball seen  dist error = " << dist_error
      << "  dir error = " << dir_error << ENDL);
    */
    ////////////////////////////////////////////////////////////////////
    // get coordinate
    double ave_cos = AngleDeg::cos_deg( average_dir );
    double ave_sin = AngleDeg::sin_deg( average_dir );

    rpos->x = average_dist * ave_cos;
    rpos->y = average_dist * ave_sin;

    // get coordinate error
    double mincos = AngleDeg::cos_deg( average_dir - dir_error );
    double maxcos = AngleDeg::cos_deg( average_dir + dir_error );
    double minsin = AngleDeg::sin_deg( average_dir - dir_error );
    double maxsin = AngleDeg::sin_deg( average_dir + dir_error );


#if 0
    std::vector< double > xvec, yvec;
    xvec.push_back( max_dist * mincos );
    xvec.push_back( max_dist * maxcos );
    xvec.push_back( min_dist * mincos );
    xvec.push_back( min_dist * maxcos );

    yvec.push_back( max_dist * minsin );
    yvec.push_back( max_dist * maxsin );
    yvec.push_back( min_dist * minsin );
    yvec.push_back( min_dist * maxsin );

    rpos_err->x = ( *std::max_element( xvec.begin(), xvec.end() )
                    - *std::min_element( xvec.begin(), xvec.end() ) ) * 0.5;
    rpos_err->y = ( *std::max_element( yvec.begin(), xvec.end() )
                    - *std::min_element( yvec.begin(), yvec.end() ) ) * 0.5;
#else
    double x1 = max_dist * mincos; double x2 = max_dist * maxcos;
    double x3 = min_dist * mincos; double x4 = min_dist * maxcos;

    double y1 = max_dist * minsin; double y2 = max_dist * maxsin;
    double y3 = min_dist * minsin; double y4 = min_dist * maxsin;

    rpos_err->x = ( max( max( x1, x2 ), max( x3, x4 ) )
                    - min( min( x1, x2 ), min( x3, x4 ) ) ) * 0.5;
    rpos_err->y = ( max( max( y1, y2 ), max( y3, y4 ) )
                    - min( min( y1, y2 ), min( y3, y4 ) ) ) * 0.5;
#endif

#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  __FILE__" (localizeBallRelative) Seen relative ball. ave_dist=%.1f ave_aangle=%.1f"
                  " pos = (%.3f %.3f) err = (%.3f %.3f)",
                  average_dist, average_dir,
                  rpos->x, rpos->y,
                  rpos_err->x, rpos_err->y );
#endif

    ////////////////////////////////////////////////////////////////////
    // get velocity
    if ( ball.has_vel_ )
    {
        double max_dist_dist_chg1
            = ( ball.dist_chng_ / ball.dist_ + 0.02*0.5 ) * max_dist;
        double max_dist_dist_chg2
            = ( ball.dist_chng_ / ball.dist_ - 0.02*0.5 ) * max_dist;

        double min_dist_dist_chg1
            = ( ball.dist_chng_ / ball.dist_ + 0.02*0.5 ) * min_dist;
        double min_dist_dist_chg2
            = ( ball.dist_chng_ / ball.dist_ - 0.02*0.5 ) * min_dist;

        // qstep_dir = 0.1
        double max_dir_chg = ball.dir_chng_ + ( 0.1 * 0.5 );
        double min_dir_chg = ball.dir_chng_ - ( 0.1 * 0.5 );

        double max_dist_dir_chg_r1 = AngleDeg::DEG2RAD * max_dir_chg * max_dist;
        double max_dist_dir_chg_r2 = AngleDeg::DEG2RAD * min_dir_chg * max_dist;

        double min_dist_dir_chg_r1 = AngleDeg::DEG2RAD * max_dir_chg * min_dist;
        double min_dist_dir_chg_r2 = AngleDeg::DEG2RAD * min_dir_chg * min_dist;

        // relative vel pattern : max_dist case
        Vector2D rvel1_1( max_dist_dist_chg1, max_dist_dir_chg_r1 ); rvel1_1.rotate( max_ang );
        Vector2D rvel1_2( max_dist_dist_chg1, max_dist_dir_chg_r1 ); rvel1_2.rotate( min_ang );
        Vector2D rvel2_1( max_dist_dist_chg1, max_dist_dir_chg_r2 ); rvel2_1.rotate( max_ang );
        Vector2D rvel2_2( max_dist_dist_chg1, max_dist_dir_chg_r2 ); rvel2_2.rotate( min_ang );
        Vector2D rvel3_1( max_dist_dist_chg2, max_dist_dir_chg_r1 ); rvel3_1.rotate( max_ang );
        Vector2D rvel3_2( max_dist_dist_chg2, max_dist_dir_chg_r1 ); rvel3_2.rotate( min_ang );
        Vector2D rvel4_1( max_dist_dist_chg2, max_dist_dir_chg_r2 ); rvel4_1.rotate( max_ang );
        Vector2D rvel4_2( max_dist_dist_chg2, max_dist_dir_chg_r2 ); rvel4_2.rotate( min_ang );
        // relative vel pattern : min_dist case
        Vector2D rvel5_1( min_dist_dist_chg1, min_dist_dir_chg_r1 ); rvel5_1.rotate( max_ang );
        Vector2D rvel5_2( min_dist_dist_chg1, min_dist_dir_chg_r1 ); rvel5_2.rotate( min_ang );
        Vector2D rvel6_1( min_dist_dist_chg1, min_dist_dir_chg_r2 ); rvel6_1.rotate( max_ang );
        Vector2D rvel6_2( min_dist_dist_chg1, min_dist_dir_chg_r2 ); rvel6_2.rotate( min_ang );
        Vector2D rvel7_1( min_dist_dist_chg2, min_dist_dir_chg_r1 ); rvel7_1.rotate( max_ang );
        Vector2D rvel7_2( min_dist_dist_chg2, min_dist_dir_chg_r1 ); rvel7_2.rotate( min_ang );
        Vector2D rvel8_1( min_dist_dist_chg2, min_dist_dir_chg_r2 ); rvel8_1.rotate( max_ang );
        Vector2D rvel8_2( min_dist_dist_chg2, min_dist_dir_chg_r2 ); rvel8_2.rotate( min_ang );


        double max_x = max(max(max(max(rvel1_1.x, rvel1_2.x), max(rvel2_1.x, rvel2_2.x)),
                               max(max(rvel3_1.x, rvel3_2.x), max(rvel4_1.x, rvel4_2.x))),
                           max(max(max(rvel5_1.x, rvel5_2.x), max(rvel6_1.x, rvel6_2.x)),
                               max(max(rvel7_1.x, rvel7_2.x), max(rvel8_1.x, rvel8_2.x))));
        double max_y = max(max(max(max(rvel1_1.y, rvel1_2.y), max(rvel2_1.y, rvel2_2.y)),
                               max(max(rvel3_1.y, rvel3_2.y), max(rvel4_1.y, rvel4_2.y))),
                           max(max(max(rvel5_1.y, rvel5_2.y), max(rvel6_1.y, rvel6_2.y)),
                               max(max(rvel7_1.y, rvel7_2.y), max(rvel8_1.y, rvel8_2.y))));

        double min_x = min(min(min(min(rvel1_1.x, rvel1_2.x), min(rvel2_1.x, rvel2_2.x)),
                               min(min(rvel3_1.x, rvel3_2.x), min(rvel4_1.x, rvel4_2.x))),
                           min(min(min(rvel5_1.x, rvel5_2.x), min(rvel6_1.x, rvel6_2.x)),
                               min(min(rvel7_1.x, rvel7_2.x), min(rvel8_1.x, rvel8_2.x))));
        double min_y = min(min(min(min(rvel1_1.y, rvel1_2.y), min(rvel2_1.y, rvel2_2.y)),
                               min(min(rvel3_1.y, rvel3_2.y), min(rvel4_1.y, rvel4_2.y))),
                           min(min(min(rvel5_1.y, rvel5_2.y), min(rvel6_1.y, rvel6_2.y)),
                               min(min(rvel7_1.y, rvel7_2.y), min(rvel8_1.y, rvel8_2.y))));


        Vector2D ave_rvel = rvel1_1; ave_rvel += rvel1_2;
        ave_rvel += rvel2_1; ave_rvel += rvel2_2;
        ave_rvel += rvel3_1; ave_rvel += rvel3_2;
        ave_rvel += rvel4_1; ave_rvel += rvel4_2;
        ave_rvel += rvel5_1; ave_rvel += rvel5_2;
        ave_rvel += rvel6_1; ave_rvel += rvel6_2;
        ave_rvel += rvel7_1; ave_rvel += rvel7_2;
        ave_rvel += rvel8_1; ave_rvel += rvel8_2;

        ave_rvel /= 16.0;

        *rvel = ave_rvel;
        // gvel = rvel + myvel
        rvel_err->assign( (max_x - min_x) * 0.5,
                          (max_y - min_y) * 0.5 );
        // gvel_err = rvel_err + myvel_err
#ifdef DEBUG_PRINT
        {
            Vector2D raw_rvel( ball.dist_chng_,
                               AngleDeg::DEG2RAD * ball.dir_chng_ );
            raw_rvel.rotate( average_dir );
            dlog.addText( Logger::WORLD,
                          __FILE__" (localizeBallRelative) Seen raw relative ball vel = (%.3f %.3f)",
                          raw_rvel.x, raw_rvel.y );
        }

        dlog.addText( Logger::WORLD,
                      __FILE__" (localizeBallRelative) Seen rel ball vel = (%.3f %.3f) err = (%.3f %.3f)",
                      ave_rvel.x, ave_rvel.y,
                      rvel_err->x, rvel_err->y );
#endif
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
LocalizationDefault::localizePlayer( const WorldModel & wm,
                                     const VisualSensor::PlayerT & from,
                                     const double self_face,
                                     const double self_face_err,
                                     const Vector2D & self_pos,
                                     const Vector2D & self_vel,
                                     PlayerT * to ) const
{
    ////////////////////////////////////////////////////////////////////
    // get polar range info
    double average_dist, dist_error;
#ifdef USE_OBJECT_TABLE
    if ( ! M_impl->objectTable().getDistanceRange( wm.clientVersion(),
                                                   wm.self().viewWidth(),
                                                   from.dist_, &average_dist, &dist_error ) )
    {
        std::cerr << __FILE__ << " (localizePlayer) Unexpected player distance "
                  << from.dist_ << std::endl;
        dlog.addText( Logger::WORLD,
                      __FILE__" (localizePlayer) Unexpected player distance %f",
                      from.dist_ );
        return false;
    }
#else
    M_impl->inverseDistanceRange( wm.clientVersion(), wm.self().viewWidth().type(),
                                  from.dist_, ServerParam::i().distQuantizeStep(), &average_dist, &dist_error );
#endif

    double average_dir, dir_error;

    M_impl->getDirRange( from.dir_,
                         self_face, self_face_err,
                         &average_dir, &dir_error );


    ////////////////////////////////////////////////////////////////////
    // set player info
    to->unum_ = from.unum_;
    to->goalie_ = from.goalie_;


    ////////////////////////////////////////////////////////////////////
    // get coordinate
    to->rpos_.x = average_dist * AngleDeg::cos_deg( average_dir );
    to->rpos_.y = average_dist * AngleDeg::sin_deg( average_dir );

    // set distance error
    to->dist_error_ = dist_error;

    // set global coordinate
    to->pos_ = self_pos + to->rpos_;


    ////////////////////////////////////////////////////////////////////
    // get vel
    // use only seen info, not consider noise
    if ( from.has_vel_ )
    {
#if 1
        to->vel_.assign( from.dist_chng_,
                         AngleDeg::DEG2RAD * from.dir_chng_ * average_dist );
        to->vel_.rotate( average_dir );
        to->vel_ += self_vel;
#else
        to->vel_.x = self_vel.x + ( from.dist_chng_ * to->rpos.x / average_dist
                                    - AngleDeg::DEG2RAD * from.dir_chng_ * to->rpos.y );
        to->vel_.y = self_vel.y + ( from.dist_chng_ * to->rpos.y / average_dist
                                    + AngleDeg::DEG2RAD * from.dir_chng_ * to->rpos.x );
#endif
    }
    else
    {
        to->vel_.invalidate();
    }

    ////////////////////////////////////////////////////////////////////
    // get player body & neck global angle
    to->has_face_ = false;
    if ( from.body_ != VisualSensor::DIR_ERR
         && from.face_ != VisualSensor::DIR_ERR )
    {
        to->has_face_ = true;
        to->body_ = AngleDeg::normalize_angle( from.body_ + self_face );
        to->face_ = AngleDeg::normalize_angle( from.face_ + self_face );
    }


    ////////////////////////////////////////////////////////////////////
    // get pointto info
    to->pointto_ = false;
    if ( from.arm_ != VisualSensor::DIR_ERR )
    {
        to->pointto_ = true;
        to->arm_ = AngleDeg::normalize_angle( from.arm_ + self_face );
    }

    ////////////////////////////////////////////////////////////////////
    // get kick info
    to->kicking_ = from.kicking_;

    ////////////////////////////////////////////////////////////////////
    // get tackle info
    to->tackle_ = from.tackle_;

    return true;
}

}
