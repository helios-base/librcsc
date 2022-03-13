// -*-c++-*-

/*!
  \file formation_static.h
  \brief static type formation method classes Header File.
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

#ifndef RCSC_FORMATION_FORMATION_STATIC_H
#define RCSC_FORMATION_FORMATION_STATIC_H

#include <rcsc/formation/formation.h>
#include <rcsc/geom/vector_2d.h>

namespace rcsc {

/*!
  \class FormationStatic
  \brief static position type formation
 */
class FormationStatic
    : public Formation {
public:

    static const std::string NAME; //!< type name

private:

    //! home position for all players
    std::array< Vector2D, 11 > M_positions;

public:
    /*!
      \brief just call the base class constructor
     */
    FormationStatic();

    /*!
      \brief static factory method. create this class.
      \return pointer to the new instance
     */
    static
    Formation::Ptr create()
      {
          return Formation::Ptr( new FormationStatic() );
      }

    /*!
      \brief get the method name of the formation model
      \return name string
    */
    virtual
    std::string methodName() const override;

    /*!
      \brief get position for the current focus point
      \param unum player number
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
