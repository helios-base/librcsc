// -*-c++-*-

/*!
  \file formation.h
  \brief abstract formation classes Header File.
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

#ifndef RCSC_FORMATION_FORMATION_H
#define RCSC_FORMATION_FORMATION_H

#include <rcsc/formation/formation_data.h>
#include <rcsc/formation/role_type.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/types.h>

#include <memory>
#include <string>
#include <list>
#include <vector>
#include <iostream>

namespace rcsc {

/*!
  \class Formation
  \brief abstarct formation class
*/
class Formation {
public:

    typedef std::shared_ptr< Formation > Ptr; //<! pointer type
    typedef std::shared_ptr< const Formation > ConstPtr; //<! const pointer type

    /*!
      \brief create a formation instance based on the input name.
      \param name formation type name
      \return smart pointer to the formation instance
    */
    static
    Ptr create( const std::string & name );

protected:

    std::string M_version; //!< version string

    std::array< std::string, 11 > M_role_names; //!< role name array
    std::array< RoleType, 11 > M_role_types; //!< role type array
    std::array< int, 11 > M_position_pairs; //!< the paired position number array. 0 means no pair.


    /*!
      \brief initialize paired position number array with 0
    */
    Formation();

public:

    /*!
      \brief destructor.
    */
    virtual
    ~Formation()
    { }

    /*!
      \brief get the version string
      \return const reference to the string
     */
    const std::string & version() const
    {
        return M_version;
    }

    /*!
      \brief get the role name array
      \return const reference to the array
     */
    const std::array< std::string, 11 > & roleNames() const
    {
        return M_role_names;
    }

    /*!
      \brief get the role type array
      \return const reference to the array
     */
    const std::array< RoleType, 11 > & roleTypes() const
    {
        return M_role_types;
    }

    /*!
      \brief get the position pair array
      \return const reference to the array
     */
    const std::array< int, 11 > & positionPairs() const
    {
        return M_position_pairs;
    }

    /*!
      \brief get role name value
      \param num target position number
      \return role name string
    */
    std::string roleName( const int num ) const
    {
        return ( num < 1 || 11 < num
                 ? std::string()
                 : M_role_names[num - 1] );
    }

    /*!
      \brief get role type value
      \param num target player's position number
      \return role type
    */
    RoleType roleType( const int num ) const
    {
        return ( num < 1 || 11 < num
                 ? RoleType()
                 : M_role_types[num - 1] );
    }

    /*!
      \brief get paired position number
      \param num target position number
      \retrun position number
    */
    int pairedNumber( const int num ) const
    {
        return ( num < 1 || 11 < num
                 ? 0
                 : M_position_pairs[num - 1] );
    }

public:

    /*!
      \brief set the version string
      \param ver version string
      \return true if success
    */
    bool setVersion( const std::string & ver );

    /*!
      \brief set the role name
      \param num position number
      \param name role name string
      \return true if success
    */
    bool setRoleName( const int num,
                      const std::string & name );

    /*!
      \brief set the role type
      \param num position number
      \param type role type value
      \return true if success
    */
    bool setRoleType( const int num,
                      const RoleType & type );

    /*!
      \brief set the position pair
      \param num position number
      \param paired_num pared player's uniform number
      \return true if success
    */
    bool setPositionPair( const int num,
                          const int paired_num );

    /*!
      \brief set role data
      \param num position number
      \param type role type info
      \param paired_num paired position number
      \return true if success
    */
    bool setRole( const int num,
                  const std::string & name,
                  const RoleType & type,
                  const int paired_num );

    /*!
      \brief get the method name of the formation model
      \return name string
    */
    virtual
    std::string methodName() const = 0;

    /*!
      \brief get position for the current focus point
      \param num player number
      \param focus_point current focus point, usually ball position.
    */
    virtual
    Vector2D getPosition( const int num,
                          const Vector2D & focus_point ) const = 0;

    /*!
      \brief get all positions for the current focus point
      \param focus_point current focus point, usually ball position
      \param positions contaner to store the result
    */
    virtual
    void getPositions( const Vector2D & focus_point,
                       std::vector< Vector2D > & positions ) const = 0;

    /*!
      \brief update formation paramter using training data set
      \param data training data
      \return true if success
    */
    virtual
    bool train( const FormationData & data ) = 0;

    /*!
      \brief create data for the editor
      \return formation data
     */
    virtual
    FormationData::Ptr toData() const = 0;

    /*!
      \brief print formation model to the output stream
      \param os output stream
      \return true if success
     */
    bool print( std::ostream & os ) const;

protected:
    /*!
      \brief print version string
      \param os output stream
      \return true if success
     */
    bool printVersion( std::ostream & os ) const;

        /*!
      \brief print method name
      \param os output stream
      \return true if success
     */
    bool printMethodName( std::ostream & os ) const;

    /*!
      \brief print role array
      \param os output stream
      \return true if success
     */
    bool printRoles( std::ostream & os ) const;

    /*!
      \brief print model data
      \param os output stream
      \return true if success
     */
    virtual
    bool printData( std::ostream & os ) const = 0;
};

}

#endif
