// -*-c++-*-

/*!
  \file formation_cdt.h
  \brief formation using Constrained Delaunay Triangulation Header File.
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

#ifndef RCSC_FORMATION_FORMATION_CDT_H
#define RCSC_FORMATION_FORMATION_CDT_H

#include <rcsc/formation/formation.h>
#include <rcsc/geom/triangulation.h>
#include <iostream>

namespace rcsc {

/*!
  \class FormationCDT
  \brief formation which utilizes Constrained Delaunay Triangulation
*/
class FormationCDT
    : public Formation {
public:

    static const std::string NAME; //!< type name

private:

    //! player's role names
    std::string M_role_name[11];

    //! set of desired positins used by delaunay triangulation & linear interpolation
    std::vector< formation::SampleData > M_sample_vector;

    //! constrained delaunay triangulation
    Triangulation M_triangulation;

public:

    /*!
      \brief just call the base class constructor to initialize formation method name
    */
    FormationCDT();


    /*!
      \brief static method. get formation method name
      \return method name string
    */
    static
    std::string name()
      {
          return NAME;
      }

    /*!
      \brief static factory method. create new object
      \return new object
    */
    static
    Formation::Ptr create()
      {
          return Formation::Ptr( new FormationCDT() );
      }

    /*!
      \brief get the delaunay triangulation
      \return const reference to the triangulation instance
     */
    const
    Triangulation & triangulation() const
      {
          return M_triangulation;
      }

    //--------------------------------------------------------------

    /*!
      \brief create default formation. assign role and initial positions.
      \return snapshow variable for the initial state(ball pos=(0,0)).
    */
    virtual
    void createDefaultData();

    /*!
      \brief get the name of this formation
      \return name string
    */
    virtual
    std::string methodName() const
      {
          return FormationCDT::name();
      }

protected:
    /*!
      \brief create new role parameter.
      \param unum target player's number
      \param role_name new role name
      \param type side type of this parameter
    */
    virtual
    void createNewRole( const int unum,
                        const std::string & role_name,
                        const SideType type );
    /*!
      \brief set the role name of the specified player
      \param unum target player's number
      \param name role name string.
    */
    virtual
    void setRoleName( const int unum,
                      const std::string & name );

public:

    /*!
      \brief get the role name of the specified player
      \param unum target player's number
      \return role name string. if empty string is returned,
      that means no role parameter is assigned for unum.
    */
    virtual
    std::string getRoleName( const int unum ) const;

    /*!
      \brief get position for the current focus point
      \param unum player number
      \param focus_point current focus point, usually ball position.
    */
    virtual
    Vector2D getPosition( const int unum,
                          const Vector2D & focus_point ) const;

    /*!
      \brief get all positions for the current focus point
      \param focus_point current focus point, usually ball position
      \param positions contaner to store the result
     */
    virtual
    void getPositions( const Vector2D & focus_point,
                       std::vector< Vector2D > & positions ) const;

    /*!
      \brief update formation paramter using training data set
    */
    virtual
    void train();

    /*!
      \brief read formation data from the input stream.
      \param is reference to the input stream.
      \return result status.
    */
    virtual
    bool read( std::istream & is );

    /*!
      \brief put formation data to the output stream.
      \param os reference to the output stream
      \return reference to the output stream
    */
    virtual
    std::ostream & print( std::ostream & os ) const;

private:

    Vector2D interpolate( const int unum,
                          const Vector2D & focus_point,
                          const Triangulation::Triangle * tri ) const;


protected:

    /*!
      \brief restore conf data from the input stream.
      \param is reference to the input stream.
      \return parsing result
    */
    virtual
    bool readConf( std::istream & is );

    /*!
      \brief read lines until the 'End' line found.
      \param is input stream
      \return parsing result
     */
    bool readEnd( std::istream & is );

    /*!
      \brief reconstruct model using read samples
      \return result of reconstruction
     */
    virtual
    bool generateModel();

    /*!
      \brief put data to the output stream.
      \param os reference to the output stream
      \return reference to the output stream
    */
    virtual
    std::ostream & printConf( std::ostream & os ) const;

    /*!
      \brief put the last info if necessary
      \param os reference to the output stream
      \return reference to the output stream
    */
    virtual
    std::ostream & printEnd( std::ostream & os ) const;

private:

    /*!
      \brief restore role assignment from the input stream
      \param is reference to the input stream
      \return parsing result
    */
    bool readRoles( std::istream & is );

    /*!
      \brief print role data to the output stream.
      \param os reference to the output stream.
      \return reference to the output stream.
     */
    std::ostream & printRoles( std::ostream & os ) const;

};

}

#endif
