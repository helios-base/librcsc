// -*-c++-*-

/*!
  \file formation_sbsp.h
  \brief simple SBSP formation Header File.
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

#ifndef RCSC_FORMATION_FORMATION_SBSP_H
#define RCSC_FORMATION_FORMATION_SBSP_H

#include <rcsc/formation/formation.h>
#include <rcsc/geom/rect_2d.h>

#include <array>

namespace rcsc {

/*!
  \class FormationSBSP
  \brief formation implementation using SBSP method
*/
class FormationSBSP
    : public Formation {
public:

    static const std::string NAME; //!< type name

    /*!
      \struct Role
      \brief role parameter
     */
    struct Role {
        int number_; //!< player number
        int symmetry_; //!< mirror reference nuber. =0:center, -1:side, >0:refered number
        std::string name_; //!< role name string
        Vector2D pos_; //!< basic position
        Vector2D attract_; //!< attraction parameter
        Rect2D region_; //!< movable area
        bool behind_ball_; //!< defensive flag

        /*!
          \brief default constructor
         */
        Role();

        /*!
          \brief construct with parameters
          \param attract attraction parameter vector
          \param region movable region
          \param behind_ball behind flag
         */
        Role( const Vector2D & attract,
              const Rect2D & region,
              const bool behind_ball );

        /*!
          \brief create random parameters
         */
        void randomize();

        /*!
          \brief read data from an input stream
          \return parsing result
         */
        bool read( std::istream & is );

        /*!
          \brief write data to an output stream
          \param os reference to the output stream
          \return reference to the output stream
         */
        std::ostream & print( std::ostream & os ) const;
    };

    /*!
      \class Param
      \brief the set of role
     */
    class Param {
    private:
        std::string M_name; //!< formation name
        std::array< Role, 11 > M_roles; //!< role set

        Param() = delete; // not used
    public:

        /*!
          \brief construct with formation name
          \param name formation name
         */
        explicit
        Param( const std::string & name )
            : M_name( name )
          { }

        /*!
          \brief get the symmetry information of the specified player
          \param unum target player number
         */
        int getSymmetry( const int unum ) const
          {
              return M_roles.at( unum - 1 ).symmetry_;
          }

        /*!
          \brief get the role of the specified number
          \param unum target player number
         */
        Role & getRole( const int unum )
          {
              return M_roles.at( unum - 1 );
          }

        /*!
          \brief get the role of the specified number
          \param unum target player number
         */
        const
        Role & getRole( const int unum ) const
          {
              return M_roles.at( unum - 1 );
          }

        /*!
          \brief get the position for the input
          \param unum target player number
          \param ball_pos input ball position
          \return player's move position
         */
        Vector2D getPosition( const int unum,
                              const Vector2D & ball_pos ) const;

        /*!
          \brief check the validity of this formation
          \return true if this is valid formation.
         */
        bool check();

        /*!
          \brief set synmetric parameters
         */
        void createSymmetryParam();

        /*!
          \brief read data from an input stream
          \return parsing result
         */
        bool read( std::istream & is );

        /*!
          \brief write data to an output stream
          \param os reference to the output stream
          \return reference to the output stream
         */
        std::ostream & print( std::ostream & os ) const;

    };


private:

    Param M_param;

public:

    /*!
      \brief just call the base class constructor
    */
    FormationSBSP();

    /*!
      \brief static method. get the type name of this formation
      \return type name string
    */
    static
    std::string name()
      {
          //return std::string( "SBSP" );
          return NAME;
      }

    /*!
      \brief static method. factory of this class
      \return pointer to the new object
    */
    static
    Formation::Ptr create()
      {
          return Formation::Ptr( new FormationSBSP() );
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
          return FormationSBSP::name();
      }

protected:

    /*!
      \brief set the role name of the specified player
      \param unum target player's number
      \param name role name string.
    */
    virtual
    void setRoleName( const int unum,
                      const std::string & name );

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
      \param ball_pos current focus point, usually ball position.
    */
    virtual
    Vector2D getPosition( const int unum,
                          const Vector2D & ball_pos ) const;

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
      \brief get the current formation parameter
      \return const reference to the current formation parameter
    */
    const
    Param & param() const
      {
          return M_param;
      }
};

}

#endif
