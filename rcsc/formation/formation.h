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

#include <rcsc/formation/sample_data.h>
#include <rcsc/common/role_type.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/factory.h>
#include <rcsc/types.h>

#include <boost/shared_ptr.hpp>

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

    typedef boost::shared_ptr< Formation > Ptr; //<! pointer type
    typedef boost::shared_ptr< const Formation > ConstPtr; //<! const pointer type
    typedef Ptr (*Creator)(); //!< creator function
    typedef rcss::Factory< Creator, std::string > Creators; //!< creator function holder

    /*!
      \enum SideType
      \brief enumeration for the side type in formation
     */
    enum SideType {
        SIDE = -1, //!< original type, consider all region
        SYMMETRY = 1, //!< symmetry type, this type refers SIDE
        CENTER = 0, //!< original type, consider half region
    };

    /*!
      \brief creator holder singleton
      \return reference to the creator holder instance
     */
    static
    Creators & creators();


    /*!
      \brief create a formation instance based on the input name.
      \param name formation type name
      \return smart pointer to the formation instance
     */
    static
    Ptr create( const std::string & name );

    /*!
      \brief create a formation instance based on the input stream.
      \param is reference to the input stream (usually fstream).
      \return smart pointer to the formation instance
     */
    static
    Ptr create( std::istream & is );

protected:

    /*!
      \brief data format version.
     */
    int M_version;

    RoleType M_role_type[11];

    /*!
      \brief symmetry number holder.
      negative number means this player is original SIDE type.
      positive numver means that this player is SYMMETRY type and
      referes other player.
      Zero means that this player is CENTER type.
    */
    int M_symmetry_number[11];

    bool M_marker[11]; //!< switch that indicates marker player or not
    bool M_setplay_marker[11]; //!< switch that indicates setplay marker player or not

    /*!
      \brief training data holder.
     */
    formation::SampleDataSet::Ptr M_samples;

public:

    /*!
      \brief initialize symmetry number matrix by -1
    */
    Formation();

    /*!
      \brief destructor.
     */
    virtual
    ~Formation()
      { }

    /*!
      \brief create default formation. assign role and initial positions.
      \return snapshot variable for the initial state(ball pos=(0,0)).
    */
    virtual
    void createDefaultData() = 0;

    /*!
      \brief get data format version.
      \return data format version.
     */
    int version() const
      {
          return M_version;
      }

    /*!
      \brief get the formation method type name.
      \return name string
    */
    virtual
    std::string methodName() const = 0;

    /*!
      \brief get sample point set.
      \return shared pointer to the training data object.
     */
    formation::SampleDataSet::Ptr samples()
      {
          return M_samples;
      }

    /*!
      \brief get sample point set.
      \return shared pointer to the training data object.
     */
    formation::SampleDataSet::ConstPtr samples() const
      {
          return M_samples;
      }

    /*!
      \brief set new sample point set.
      \param samples pointer to the new data instance.
     */
    void setSamples( formation::SampleDataSet::Ptr samples )
      {
          M_samples = samples;
      }

    //
    //
    //

    /*!
      \brief get role type value
      \param unum target player's uniform number
      return role type
     */
    RoleType roleType( const int unum ) const
      {
          if ( unum < 1 || 11 < unum ) return RoleType();
          return M_role_type[unum-1];
      }

    /*!
      \brief check if player is SIDE type or not
      \param unum player's number
      \return true or false
    */
    bool isSideType( const int unum ) const
      {
          if ( unum < 1 || 11 < unum ) return false;
          return ( M_symmetry_number[unum - 1] < 0 );
      }

    /*!
      \brief check if player is center type or not
      \param unum player's number
      \return true or false
    */
    bool isCenterType( const int unum ) const
      {
          if ( unum < 1 || 11 < unum ) return false;
          return ( M_symmetry_number[unum - 1] == 0 );
      }

    /*!
      \brief check if player is right side type or not
      \param unum player's number
      \return true or false
    */
    bool isSymmetryType( const int unum ) const
      {
          if ( unum < 1 || 11 < unum ) return false;
          return ( M_symmetry_number[unum - 1] > 0 );
      }

    /*!
      \brief get symmetry reference number of the specified player.
      \param unum target player's number
      \return number that player refers, if player is SYMMETRY type.
      otherwise 0 or -1.
    */
    int getSymmetryNumber( const int unum ) const
      {
          if ( unum < 1 || 11 < unum ) return 0;
          return M_symmetry_number[unum - 1];
      }

    /*!
      \brief check whether the target player is marker type or not
      \param unum target player's uniform number
      \return true if the player is marker type
     */
    bool isMarker( const int unum ) const
      {
          if ( unum < 1 || 11 < unum ) return false;
          return M_marker[unum - 1];
      }

    /*!
      \brief check whether the target player is marker type or not
      \param unum target player's uniform number
      \return true if the player is marker type
     */
    bool isSetPlayMarker( const int unum ) const
      {
          if ( unum < 1 || 11 < unum ) return false;
          return M_setplay_marker[unum - 1];
      }

    //--------------------------------------------------------------
    // APIs for formation editor

    /*!
      \brief set player's role data. if necessary, new parameter is created.
      \param unum status changed player's uniform number
      \param symmetry_unum 0 means type is CENTER, <0 means type is SIDE,
      >0 means type is SYMMETRY
      \param role_name new role name string
      \return true if successfully updated
    */
    bool updateRole( const int unum,
                     const int symmetry_unum,
                     const std::string & role_name );

    /*!
      \brief set player's role type
      \param unum player's uniform number
      \param type role type
      \return true if successfully updated
    */
    bool updateRoleType( const int unum,
                         const RoleType::Type type );

    /*!
      \brief set marker switch for each player
      \param unum target player's uniform number
      \param marker swtich for play_on period
      \param setplay_marker swtich for set_play period
      \return true if successfully updated
     */
    bool updateMarker( const int unum,
                       const bool marker,
                       const bool setplay_marker );

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
                        const SideType type ) = 0;

    /*!
      \brief set the role name of the specified player
      \param unum target player's number
      \param name role name string.
    */
    virtual
    void setRoleName( const int unum,
                      const std::string & name ) = 0;

    /*!
      \brief set role type to the target player
      \param unum player's uniform number
      \param type role type string: "G", "DF", "MF" or "FW"
    */
    void setRoleType( const int unum,
                      const std::string & role_type );

    /*!
      \brief set the specified player to the CENTER type
      \param unum player's number
    */
    void setCenterType( const int unum );

    /*!
      \brief set the specified player to the SIDE type
      \param unum player's number
    */
    void setSideType( const int unum );

    /*!
      \brief set symmetry player info
      \param unum changed player's number
      \param symmetry_unum refered player number
    */
    bool setSymmetryType( const int unum,
                          const int symmetry_unum,
                          const std::string & role_name );

    /*!
      \brief set marker type
      \param unum player's number
      \param marker "marker" if the target player is marker type
      \param marker "setplay_marker" if the target player is marker type
     */
    void setMarker( const int unum,
                    const std::string & marker,
                    const std::string & smarker );
public:

    /*!
      \brief get the role name of the specified player
      \param unum target player's number
      \return role name string. if empty string is returned,
      that means no role parameter is assigned for unum.
    */
    virtual
    std::string getRoleName( const int unum ) const = 0;

    /*!
      \brief get position for the current focus point
      \param unum player number
      \param focus_point current focus point, usually ball position.
    */
    virtual
    Vector2D getPosition( const int unum,
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
    */
    virtual
    void train() = 0;

    /*!
      \brief read formation data from the input stream.
      \param is reference to the input stream.
      \return result status.
    */
    virtual
    bool read( std::istream & is ) = 0;
    virtual
    bool readOld( std::istream & ) = 0;
    virtual
    bool readCSV( std::istream & ) = 0;

    /*!
      \brief put formation data to the output stream.
      \param os reference to the output stream
      \return reference to the output stream
    */
    virtual
    std::ostream & print( std::ostream & os ) const = 0;
    virtual
    std::ostream & printOld( std::ostream & os ) const = 0;
    virtual
    std::ostream & printCSV( std::ostream & os ) const = 0;


    /*!
      \brief put comment line
      \param os reference to the output stream
      \param msg comment message
      \return reference to the output stream
    */
    std::ostream & printComment( std::ostream & os,
                                 const std::string & msg ) const;

protected:

    //
    // read
    //

    /*!
      \brief read header information (formation type name, format version...)
      \param is reference to the input stream
      \return result status.
    */
    bool readHeader( std::istream & is );
    bool readMethodName( std::istream & is );

    /*!
      \brief read sample point data from the input stream.
      \param is reference to the input stream.
      \return result status.
    */
    bool readSamplesOld( std::istream & is );
    bool readSamplesCSV( std::istream & is );

    /*!
      \brief check the consistency of symetric unum reference in read data
      \return checked result
     */
    bool checkSymmetryNumber() const;

    //
    // print
    //

    /*!
      \brief put header information to the output stream
      \param os reference to the output stream
      \return reference to the output stream
    */
    std::ostream & printHeader( std::ostream & os ) const;
    std::ostream & printMethodName( std::ostream & os ) const;

    /*!
      \brief put sample point data to the output stream.
      \param os reference to the output stream
      \return reference to the output stream
    */
    std::ostream & printSamplesOld( std::ostream & os ) const;
    std::ostream & printSamplesCSV( std::ostream & os ) const;
};

}

#endif
