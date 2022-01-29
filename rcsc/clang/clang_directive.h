// -*-c++-*-

/*!
  \file clang_directive.h
  \brief clang directive class Header File
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

#ifndef RCSC_CLANG_DIRECTIVE_H
#define RCSC_CLANG_DIRECTIVE_H

#include <rcsc/clang/clang_action.h>

#include <memory>
#include <vector>
#include <iosfwd>

namespace rcsc {

/*!
  \class CLangDirective
  \brief abstract clang directive
 */
class CLangDirective {
public:

    /*!
      \enum Type
      \brief clang directive types
     */
    enum Type {
        COMMON,
        NAMED,
        MAX_TYPE,
    };

    //! smart pointer type
    typedef std::shared_ptr< const CLangDirective > ConstPtr;

    //! directive list type
    typedef std::vector< ConstPtr > Cont;

protected:

    /*!
      \brief protected constructor
     */
    CLangDirective()
      { }

public:

    /*!
      \brief virtual destructor
     */
    virtual
    ~CLangDirective()
      { }

    /*!
      \brief get type id.
      \return action type id.
     */
    virtual
    Type type() const = 0;

    /*!
      \brief print clang message to the output stream
      \param os reference to the output stream
      \return reference to the output stream
     */
    virtual
    std::ostream & print( std::ostream & os ) const = 0;

};

}

/*!
  \brief stream operator
  \param os reference to the output stream
  \param tok clang token object
  \return reference to the output stream
 */
inline
std::ostream &
operator<<( std::ostream & os,
            const rcsc::CLangDirective & dir )
{
    return dir.print( os );
}

namespace rcsc {

class CLangDirectiveCommon
    : public CLangDirective {
private:

    bool M_positive; //!< positive flag (do or dont)
    bool M_our; //!< side flag
    CLangUnumSet::Ptr M_players; //!< target players
    CLangAction::Cont M_actions; //!< actions

public:

    CLangDirectiveCommon()
        : M_positive( false ),
          M_our( false )
      { }

    // ~CLangDirectiveCommon()
    //   {
    //       std::cerr << "delete CLangDirectiveCommon " << *this << std::endl;
    //   }

    /*!
      \brief get type id.
      \return action type id.
     */
    virtual
    Type type() const
      {
          return COMMON;
      }

    /*!
      \brief set positive flag (do or dont)
      \param positive positive flag
     */
    void setPositive( const bool positive )
      {
          M_positive = positive;
      }

    /*!
      \brief set side flag.
      \param our side flag.
     */
    void setOur( const bool our )
      {
          M_our = our;
      }

    /*!
      \brief set target players.
      \param players new target player set
     */
    void setPlayers( CLangUnumSet * players )
      {
          M_players = CLangUnumSet::Ptr( players );
      }

    /*!
      \brief add target player
      \param unum target player's uniform number
     */
    void addPlayer( const int unum )
      {
          if ( ! M_players ) M_players = CLangUnumSet::Ptr( new CLangUnumSet() );
          M_players->add( unum );
      }

    /*!
      \brief add new action.
      \param act pointer to the dynamically allocated action object
     */
    void addAction( CLangAction * act )
      {
          M_actions.push_back( CLangAction::ConstPtr( act ) );
      }

    //
    //
    //

    /*!
      \brief check if this directive is positive type (do) or not.
      \return true if positive type
     */
    bool isPositive() const
      {
          return M_positive;
      }

    /*!
      \brief check if this directive is negative type (dont) or not.
      \return true if negative type
     */
    bool isNegative() const
      {
          return ! isPositive();
      }

    /*!
      \brief check if this directive target is our team or not.
      \return checked result
     */
    bool isOur() const
      {
          return M_our;
      }

    /*!
      \brief check if this directive target is their team or not.
      \return checked result
     */
    bool isTheir() const
      {
          return ! isOur();
      }

    /*!
      \brief get the unum set of target players
      \return unum set
     */
    const CLangUnumSet::Ptr & players() const
      {
          return M_players;
      }

    /*!
      \brief get action container.
      \return action container.
     */
    const CLangAction::Cont & actions() const
      {
          return M_actions;
      }

    virtual
    std::ostream & print( std::ostream & os ) const;

};

//
// TODO:
//
// named directive
//

}

#endif
