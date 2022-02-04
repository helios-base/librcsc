// -*-c++-*-

/*!
  \file formation_uva.h
  \brief UvA Trilearn type formation method classes Header File.
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

#ifndef RCSC_FORMATION_FORMATION_UVA_H
#define RCSC_FORMATION_FORMATION_UVA_H

#include <iostream>
#include <map>

#include <rcsc/geom/vector_2d.h>
#include <rcsc/formation/formation.h>

namespace rcsc {

/*!
  \class FormationUvA
  \brief uva trilearn type formation
 */
class FormationUvA
    : public Formation {
public:

    static const std::string NAME; //!< type name

    /*!
      \class RoleParam
      \brief This class contains information for one individual role.
    */
    class RoleParam {
        std::string M_name; //!< role name
        double M_attr_x; //!< x attraction to the ball
        double M_attr_y; //!< y attraction to the ball
        bool M_behind_ball; //!< should player always stay behind the ball
        double M_min_x; //!< minimal x coordinate for this player type
        double M_max_x; //!< maximal x coordinate for this player type

    public:

        /*!
          \brief construct empty parameter. all values are set to 0.
        */
        RoleParam();

        /*!
          \brief construct with all values.
          \param name role name
          \param attr_x x attraction to the ball
          \param attr_y y attraction to the ball
          \param behind_ball if true, player should always stay behind the ball.
          \param min_x minimal x coordinate
          \param max_x maximal x coordinate
        */
        RoleParam( const std::string & name,
                   const double & attr_x,
                   const double & attr_y,
                   const bool behind_ball,
                   const double & min_x,
                   const double & max_x );

        /*!
          \brief set all variables
          \param name role name
          \param attr_x x attraction to the ball
          \param attr_y y attraction to the ball
          \param behind_ball if true, player should always stay behind the ball.
          \param min_x minimal x coordinate
          \param max_x maximal x coordinate
          \return const reference to itself
        */
        const
        RoleParam & assign( const std::string & name,
                            const double & attr_x,
                            const double & attr_y,
                            const bool behind_ball,
                            const double & min_x,
                            const double & max_x );

        /*!
          \brief get role name
          \return role name string
         */
        const
        std::string & name() const
          {
              return M_name;
          }

        /*!
          \brief get x attraction to the ball
          \return attraction value
         */
        const
        double & attrX() const
          {
              return M_attr_x;
          }

        /*!
          \brief get y attraction to the ball
          \return attraction value
         */
        const
        double & attrY() const
          {
              return M_attr_y;
          }

        /*!
          \brief get the switch of behind ball action
          \return switch value
         */
        bool behindBall() const
          {
              return M_behind_ball;
          }

        /*!
          \brief get the minimal x coordinate
          \return x coordinate value
         */
        const
        double & minX() const
          {
              return M_min_x;
          }

        /*!
          \brief get the maximal x coordinate
          \return x coordinate value
         */
        const
        double & maxX() const
          {
              return M_max_x;
          }

        /*!
          \brief put all variables to output stream
          \param os reference to the output stream
          \return reference to the output stream
         */
        std::ostream & print( std::ostream & os ) const;
    };


private:
    std::string M_role_names[11]; //!< role names
    Vector2D M_home_pos[11]; //!< home position for roles
    std::map< std::string, RoleParam > M_role_params; //! key: role name, value role parameter
    double M_max_y_percentage; //!< the rate of maximum y coordinate in the field coordinate system
public:
    /*!
      \brief just call the base class constructor
     */
    FormationUvA();

    /*!
      \brief static method. get the type name of this formation
      \return type name string
     */
    static
    std::string name()
      {
          //return std::string( "UvA" );
          return NAME;
      }

    /*!
      \brief static factory method. create this class.
      \return pointer to the new instance
     */
    static
    Formation::Ptr create()
      {
          return Formation::Ptr( new FormationUvA() );
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
          return FormationUvA::name();
      }

    /*!
      \brief get the maximal y percentage of players' position
      \return y percentage value [0,1]
     */
    const
    double & maxYPercentage() const
      {
          return M_max_y_percentage;
      }

    /*!
      \brief set the maximal y percentage of players' position.
      \param value new percentage value. should be [0,1]
      \return old percentage value
     */
    double setMaxYPercentage( const double & value )
      {
          double old_value = M_max_y_percentage;
          M_max_y_percentage = value;
          return old_value;
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
      \brief put all variables to output stream
      \param os reference to the output stream
      \return  reference to the output stream
     */
    virtual
    std::ostream & printConf( std::ostream & os ) const;

private:

    /*!
      \brief restore players from the input stream
      \param is reference to the input stream.
      \return parsing result
     */
    bool readPlayers( std::istream & is );

    /*!
      \brief restore roles from the input stream
      \param is reference to the input stream.
      \return parsing result
     */
    bool readRoles( std::istream & is );

};

}

#endif
