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

    //! player's role names
    std::string M_role_name[11];

    //! set of desired positins used by delaunay triangulation & linear interpolation
    std::vector< formation::SampleData > M_sample_vector;

    //! delaunay triangulation
    DelaunayTriangulation M_triangulation;

public:

    /*!
      \brief just call the base class constructor to initialize formation method name
    */
    FormationDT();


    /*!
      \brief static method. get formation method name
      \return method name string
    */
    static
    std::string name()
      {
          //return std::string( "DelaunayTriangulation" );
          return NAME;
      }

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
      \brief get the delaunay triangulation
      \return const reference to the triangulation instance
     */
    const
    DelaunayTriangulation & triangulation() const
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
          return FormationDT::name();
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

private:

    Vector2D interpolate( const int unum,
                          const Vector2D & focus_point,
                          const DelaunayTriangulation::Triangle * tri ) const;


protected:

    /*!
      \brief restore conf data from the input stream.
      \param is reference to the input stream.
      \return parsing result
    */
    virtual
    bool readConf( std::istream & is );

    /*!
      \brief read sample point data from the input stream.
      \param is reference to the input stream.
      \return result status.
    */
    virtual
    bool readSamples( std::istream & is );

    /*!
      \brief put data to the output stream.
      \param os reference to the output stream
      \return reference to the output stream
    */
    virtual
    std::ostream & printConf( std::ostream & os ) const;

    /*!
      \brief put sample point data to the output stream.
      \param os reference to the output stream
      \return reference to the output stream
    */
    virtual
    std::ostream & printSamples( std::ostream & os ) const;

private:

    /*!
      \brief read v1 format data.
      \param is reference to the input stream
      \return parsing result.
    */
    bool readV1( std::istream & is );

    /*!
      \brief (v1 format) restore role assignment from the input stream
      \param is reference to the input stream
      \return parsing result
    */
    bool readRoles( std::istream & is );

    /*!
      \brief (v1 format) restore kernel point and sample data from the input stream
      \param is reference to the input stream
      \return parsing result
    */
    bool readVertices( std::istream & is );


    /*!
      \brief read v1 format data.
      \param is reference to the input stream
      \return parsing result.
    */
    bool readV2( std::istream & is );

    /*!
      \brief (v2 format) restore role assignment from the input stream
      \param is reference to the input stream
      \return parsing result
    */
    bool readRolesV2( std::istream & is );

    /*!
      \brief (v2 format) restore vertex data set from the input stream
      \param is reference to the input stream
      \return parsing result
    */
    bool readVerticesV2( std::istream & is );

    /*!
      \brief (v2 format) restore one vertex data from the input stream.
      \param is reference to the input stream.
      \param data_index index number of this data.
      \return parsing result
    */
    bool readOneDataV2( std::istream & is,
                        const int data_index );

    /*!
      \brief (v1 format) print all data to the output stream
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printV1( std::ostream & os ) const;

    /*!
      \brief (v2 format) print all data to the output stream
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printV2( std::ostream & os ) const;

    /*!
      \brief (v2 format) print role data to the output stream
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printRolesV2( std::ostream & os ) const;

    /*!
      \brief (v2 format) print positional data to the output stream
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printDataV2( std::ostream & os ) const;
};

}

#endif
