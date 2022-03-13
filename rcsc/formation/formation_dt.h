// -*-c++-*-

/*!
  \file formation_dt.h
  \brief formation data classes using Delaunay Triangulation Header File.
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

#ifndef RCSC_FORMATION_FORMATION_DT_H
#define RCSC_FORMATION_FORMATION_DT_H

#include <rcsc/formation/formation.h>
#include <rcsc/geom/delaunay_triangulation.h>
#include <iostream>

namespace rcsc {

/*!
  \class FormationDT
  \brief formation which utilizes Delaunay Triangulation
*/
class FormationDT
    : public Formation {
public:

    static const std::string NAME; //!< type name

private:
    //! desired positins used by delaunay triangulation & linear interpolation
    std::vector< FormationData::Data > M_points;

    //! delaunay triangulation
    DelaunayTriangulation M_triangulation;

public:

    /*!
      \brief just call the base class constructor to initialize formation method name
    */
    FormationDT();

    /*!
      \brief static factory method. create new object
      \return new object
    */
    static
    Formation::Ptr create()
    {
        return Formation::Ptr( new FormationDT() );
    }

    /*!
      \brief get the sample data array
      \return sample data array
    */
    const std::vector< FormationData::Data > & points() const
    {
        return M_points;
    }

    /*!
      \brief get the delaunay triangulation
      \return const reference to the triangulation instance
    */
    const DelaunayTriangulation & triangulation() const
    {
        return M_triangulation;
    }

    /*!
      \brief get the method name of the formation model
      \return name string
    */
    virtual
    std::string methodName() const override;

    /*!
      \brief get position for the current focus point
      \param num position number
      \param focus_point current focus point, usually ball position.
    */
    virtual
    Vector2D getPosition( const int num,
                          const Vector2D & focus_point ) const override;

    /*!
      \brief get all positions for the current focus point
      \param focus_point current focus point, usually ball position
      \param positions contaner to store the result
    */
    virtual
    void getPositions( const Vector2D & focus_point,
                       std::vector< Vector2D > & positions ) const override;

private:

    Vector2D interpolate( const int num,
                          const Vector2D & focus_point,
                          const DelaunayTriangulation::Triangle * tri ) const;

public:

    /*!
      \brief update formation paramter using training data set
      \param data training data
      \return true if success
    */
    virtual
    bool train( const FormationData & data ) override;

    /*!
      \brief create data for the editor
      \return formation data
     */
    virtual
    FormationData::Ptr toData() const override;

protected:

    /*!
      \brief print model data
      \param os output stream
      \return true if success
     */
    virtual
    bool printData( std::ostream & os ) const override;
};

}

#endif
