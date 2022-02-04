// -*-c++-*-

/*!
  \file formation_ngnet.h
  \brief formation data classes using NGNet Header File.
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

#ifndef RCSC_FORMATION_FORMATION_NGNET_H
#define RCSC_FORMATION_FORMATION_NGNET_H

#include <rcsc/formation/formation.h>
#include <rcsc/ann/ngnet.h>

#include <map>

namespace rcsc {

/*!
  \class FormationNGNet
  \brief formation implementation using NGNet
*/
class FormationNGNet
    : public Formation {
public:

    static const std::string NAME; //!< type name

    /*!
      \brief formation parameter using NGNet
      one Param instance realizes just one player's position.
    */
    class Param {
    public:
        static const double PITCH_LENGTH; //!< field length
        static const double PITCH_WIDTH; //!< field width

    private:
        std::string M_role_name; //!< role name string
        NGNet M_net; //!< NGnet instance

    public:

        /*!
          \brief just set a learning parameter
        */
        Param();

        /*!
          \brief get assigned role name
          \return role name string const reference
        */
        const
        std::string & roleName() const
          {
              return M_role_name;
          }

        /*!
          \brief get RBF network
          \return reference to the RBF network
        */
        NGNet & getNet()
          {
              return M_net;
          }

        /*!
          \brief get RBF network
          \return const reference to the RBF network
        */
        const
        NGNet & net() const
          {
              return M_net;
          }

        /*!
          \brief get strategic position
          \param ball_pos focus point, usually ball position.
          \param type side type, if this is symmetry type, the refered param is used.
          \return coordinate value
        */
        Vector2D getPosition( const Vector2D & ball_pos,
                              const Formation::SideType type ) const;

        /*!
          \brief set role name
          \param name role name string
        */
        void setRoleName( const std::string & name )
          {
              M_role_name = name;
          }

        /*!
          \brief restore RBF network from the input stream
          \param is reference to the input stream
          \return result status of parsing
        */
        bool read( std::istream & is );

        /*!
          \brief put  structure to the output stream
          \param os reference to the output stream
          \return reference to the output stream
        */
        std::ostream & print( std::ostream & os ) const;

    private:
        /*!
          \brief called from read();
          \param is reference to the input stream
          \return result status of parsing
        */
        bool readRoleName( std::istream & is );

        /*!
          \brief called from read();
          \param is reference to the input stream
          \return result status of parsing
        */
        bool readParam( std::istream & is );

        /*!
          \brief called from print();
          \param os reference to the output stream
          \return reference to the output stream
        */
        std::ostream & printRoleName( std::ostream & os ) const;

        /*!
          \brief called from print();
          \param os reference to the output stream
          \return reference to the output stream
        */
        std::ostream & printParam( std::ostream & os ) const;

    };


private:

    //! key: unum. but size is not always 11 if symmetric player exists.
    std::map< int, std::shared_ptr< Param > > M_param_map;

public:

    /*!
      \brief just call the base class constructor
    */
    FormationNGNet();


    /*!
      \brief static method. get the type name of this formation
      \return type name string
    */
    static
    std::string name()
      {
          //return std::string( "NGNet" );
          return NAME;
      }

    /*!
      \brief static method. factory of this class
      \return pointer to the new object
    */
    static
    Formation::Ptr create()
      {
          return Formation::Ptr( new FormationNGNet() );
      }

    //--------------------------------------------------------------

    /*!
      \brief create default formation. assign role and initial positions.
      \return snapshow variable for the initial stat(ball pos=(0,0)).
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
          return FormationNGNet::name();
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

protected:

    /*!
      \brief restore conf data from the input stream.
      \param is reference to the input stream.
      \return pasing result.
    */
    virtual
    bool readConf( std::istream & is );

    /*!
      \brief put data to the output stream.
      \param os reference to the output stream
      \return reference to the output stream
    */
    virtual
    std::ostream & printConf( std::ostream & os ) const;

private:

    /*!
      \brief restore player from the input stream
      \param is reference to the input stream.
      \return parsing result
    */
    bool readPlayers( std::istream & is );

    /*!
      \brief get pointer to the specifed player's parameter
      \param unum player's number
      \return smart pointer to the parameter
    */
    std::shared_ptr< Param > getParam( const int unum );

    /*!
      \brief get const pointer to the specifed player's parameter
      \param unum player's number
      \return smart const pointer to the parameter
    */
    std::shared_ptr< const Param > param( const int unum ) const;

};

}

#endif
