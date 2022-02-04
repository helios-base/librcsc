// -*-c++-*-

/*!
  \file clang_action.h
  \brief clang action class Header File
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

#ifndef RCSC_CLANG_ACTION_H
#define RCSC_CLANG_ACTION_H

#include <rcsc/clang/clang_unum.h>

#include <memory>
#include <vector>
#include <iosfwd>

namespace rcsc {

/*!
  \class CLangAction
  \brief abstract action class
 */
class CLangAction {
public:

    /*!
      \enum Type
      \brief clang action types
     */
    enum Type {
        POS,
        HOME,
        BALLTO,
        MARK,
        MARK_LINE,
        OFFLINE,
        HTYPE,
        PASS,
        DRIBBLE,
        CLEAR,
        SHOOT,
        HOLD,
        INTERCEPT,
        TACKLE,
        MAX_TYPE,
    };

    //! smart pointer type
    typedef std::shared_ptr< CLangAction > ConstPtr;

    //! action container type
    typedef std::vector< ConstPtr > Cont;


protected:

    /*!
      \brief protected constructor
     */
    CLangAction()
      { }

public:

    /*!
      \brief virtual destructor
     */
    virtual
    ~CLangAction()
      { }


    /*!
      \brief get action type id.
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
  \param act clang action object
  \return reference to the output stream
 */
inline
std::ostream &
operator<<( std::ostream & os,
            const rcsc::CLangAction & act )
{
    return act.print( os );
}


namespace rcsc {

/*!
  \class CLangActionMark
  \brief clang action 'mark'
 */
class CLangActionMark
    : public CLangAction {
private:

    CLangUnumSet::Ptr M_target_players; //!< mark target players

public:

    /*!
      \brief create with empty target players
     */
    CLangActionMark()
        : M_target_players( new CLangUnumSet() )
      { }

    /*!
      \brief create with target players
      \param dynamically allocated object.
     */
    explicit
    CLangActionMark( CLangUnumSet * players )
        : M_target_players( players )
      { }

    // ~CLangActionMark()
    //   {
    //       std::cerr << "delete CLangActionMark " << *M_target_players << std::endl;
    //   }

    /*!
      \brief get type id.
      \return action type id.
     */
    virtual
    Type type() const
      {
          return MARK;
      }

    /*!
      \brief get the set of target players' unum
      \retur unum set
     */
    const CLangUnumSet::Ptr & targetPlayers() const
      {
          return M_target_players;
      }

    /*!
      \brief add new target player
      \param unum target player's uniform number
     */
    void addPlayer( const int unum )
      {
          M_target_players->add( unum );
      }

    /*!
      \brief print clang message to the output stream
      \param os reference to the output stream
      \return reference to the output stream
     */
    virtual
    std::ostream & print( std::ostream & os ) const;
};

/*!
  \class CLangActionHeteroType
  \brief clang action 'htype'
 */
class CLangActionHeteroType
    : public CLangAction {
private:

    int M_player_type; //!< player type id

public:

    /*!
      \brief create with unknown type.
     */
    CLangActionHeteroType()
        : M_player_type( -1 )
      { }

    /*!
      \brief create with the specified type id.
     */
    explicit
    CLangActionHeteroType( const int type )
        : M_player_type( type )
      { }

    // ~CLangActionHeteroType()
    //   {
    //       std::cerr << "delete CLangActionHeteroType " << M_player_type << std::endl;
    //   }

    /*!
      \brief get type id.
      \return action type id.
     */
    virtual
    Type type() const
      {
          return HTYPE;
      }

    /*!
      \brief get the player type id
      \return player type id
     */
    int playerType() const
      {
          return M_player_type;
      }

    /*!
      \brief set player type id.
      \param type player type id
     */
    void setPlayerType( const int type )
      {
          M_player_type = type;
      }


    /*!
      \brief print clang message to the output stream
      \param os reference to the output stream
      \return reference to the output stream
     */
    virtual
    std::ostream & print( std::ostream & os ) const;
};

/*!
  \class CLangActionHold
  \brief clang action 'hold'
 */
class CLangActionHold
    : public CLangAction {
private:

public:

    /*!
      \brief create with empty target players
    */
    CLangActionHold()
      { }

    // ~CLangActionHold()
    //   {
    //   }

    /*!
      \brief get type id.
      \return action type id.
     */
    virtual
    Type type() const
      {
          return HOLD;
      }

    /*!
      \brief print clang message to the output stream
      \param os reference to the output stream
      \return reference to the output stream
     */
    virtual
    std::ostream & print( std::ostream & os ) const;

};

/*!
  \class CLangActionBallTo
  \brief clang action 'bto'
 */
class CLangActionBallTo
    : public CLangAction {
private:

    CLangUnumSet::Ptr M_assigned_players; //!< assigned player who moves to ball

public:

    /*!
      \brief create with empty assigned players
     */
    CLangActionBallTo()
        : M_assigned_players( new CLangUnumSet() )
      { }

    /*!
      \brief create with the specified assigned player.
      \param dynamically allocated object.
     */
    explicit
    CLangActionBallTo( CLangUnumSet * players )
        : M_assigned_players( players )
      { }

    // ~CLangActionBallTo()
    //   {
    //       std::cerr << "delete CLangActionBallTo " << M_player_unum << std::endl;
    //   }

    /*!
      \brief get type id.
      \return action type id.
     */
    virtual
    Type type() const
      {
          return BALLTO;
      }

    /*!
      \brief get the player unum
      \return player unum
     */
    const CLangUnumSet::Ptr & assignedPlayers() const
      {
          return M_assigned_players;
      }

    /*!
      \brief set player type id.
      \param type player type id
     */
    void addPlayer( const int unum )
      {
          M_assigned_players->add( unum );
      }

    /*!
      \brief print clang message to the output stream
      \param os reference to the output stream
      \return reference to the output stream
     */
    virtual
    std::ostream & print( std::ostream & os ) const;
};



//
// TODO:
//
// named
// pos, home, ball_to_region, ball_to_unum_set
// mark_line_unum_set, mark_line_region
// offside_region
// pass_region, pass_unum_set
// dribble_region, clear_region
// shoot, hold, intercept, tackle_unum_set
//

}

#endif
