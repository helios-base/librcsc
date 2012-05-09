// -*-c++-*-

/*!
  \file soccer_action.h
  \brief abstract player action Header File
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA, Hiroki SHIMORA

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

#ifndef RCSC_PLAYER_SOCCER_ACTION_H
#define RCSC_PLAYER_SOCCER_ACTION_H

#include <boost/shared_ptr.hpp>

namespace rcsc {

class PlayerAgent;

/*!
  \class AbstractAction
  \brief base class of actions
*/
class AbstractAction {
private:
    //! number of instances have been created
    static long S_action_object_counter;

    //! object ID of this action
    long M_action_object_id;

    //! not used
    AbstractAction( const AbstractAction & );
    //! not used
    AbstractAction & operator=( const AbstractAction & );

protected:
    /*!
      \brief nothing to do. but accessible only from derived
      classes.
    */
    AbstractAction()
        : M_action_object_id( AbstractAction::S_action_object_counter++ )
      { }

public:
    /*!
      \brief nothing to do, but should be virtual.
    */
    virtual
    ~AbstractAction()
      { }

    /*!
      \brief pure virtual. set command to the action effector
      \retval true if action is performed
      \retval false if action is failed or not needed.
    */
    virtual
    bool execute( PlayerAgent * agent ) = 0;

    /*!
      \brief get ID of action object to identify action instances
      \return ID of action object
    */
    long actionObjectID() const
      {
          return M_action_object_id;
      }
};

/*!
  \class BodyAction
  \brief abstract body action
*/
class BodyAction
    : public AbstractAction {
private:

    //! not used
    BodyAction( const BodyAction & );
    //! not used
    BodyAction & operator=( const BodyAction & );

protected:
    /*!
      \brief nothing to do. but accessible only from derived
      classes.
    */
    BodyAction()
      { }

public:
    /*!
      \brief nothing to do, but should be virtual.
    */
    virtual
    ~BodyAction()
      { }

    /*!
      \brief pure virtual. set command to the action effector
      \retval true if action is performed
      \retval false if action is failed or not needed.
    */
    virtual
    bool execute( PlayerAgent * agent ) = 0;
};

/////////////////////////////////////////////////////////////////////

/*!
  \class NeckAction
  \brief abstract turn neck action
*/
class NeckAction
    : public AbstractAction {
public:

    //! smart pointer type
    typedef boost::shared_ptr< NeckAction > Ptr;

private:

    //! not used
    NeckAction( const NeckAction & );
    //! not used
    NeckAction & operator=( const NeckAction & );

protected:
    /*!
      \brief nothing to do. but accessible only from derived
      classes.
    */
    NeckAction()
      { }

public:
    /*!
      \brief nothing to do, but should be virtual.
    */
    virtual
    ~NeckAction()
      { }

    /*!
      \brief pure virtual. set command to the action effector
      \retval true if action is performed
      \retval false if action is failed or not needed.
    */
    virtual
    bool execute( PlayerAgent * agent ) = 0;

    /*!
      \brief create cloned action object
      \return pointer to the cloned object instance.
    */
    virtual
    NeckAction * clone() const = 0;
};

/////////////////////////////////////////////////////////////////////

/*!
  \class ViewAction
  \brief abstract change view action
*/
class ViewAction
    : public AbstractAction {
public:

    //! smart pointer type
    typedef boost::shared_ptr< ViewAction > Ptr;

private:

    //! not used
    ViewAction( const ViewAction & );
    //! not used
    ViewAction & operator=( const ViewAction & );

protected:
    /*!
      \brief nothing to do. but accessible only from derived
      classes.
    */
    ViewAction()
      { }

public:
    /*!
      \brief nothing to do, but should be virtual.
    */
    virtual
    ~ViewAction()
      { }

    /*!
      \brief pure virtual. set command to the action effector
      \retval true if action is performed
      \retval false if action is failed or not needed.
    */
    virtual
    bool execute( PlayerAgent * agent ) = 0;

    /*!
      \brief create cloned action object
      \return pointer to the cloned object instance.
    */
    virtual
    ViewAction * clone() const = 0;
};

/////////////////////////////////////////////////////////////////////

/*!
  \class ArmAction
  \brief abstract pointto action
*/
class ArmAction
    : public AbstractAction {
public:

    //! smart pointer type
    typedef boost::shared_ptr< ArmAction > Ptr;

private:

    //! not used
    ArmAction( const ArmAction & );
    //! not used
    ArmAction & operator=( const ArmAction & );

protected:
    /*!
      \brief nothing to do. but accessible only from derived
      classes.
    */
    ArmAction()
      { }

public:
    /*!
      \brief nothing to do, but should be virtual.
    */
    virtual
    ~ArmAction()
      { }

    /*!
      \brief pure virtual. set command to the action effector
      \retval true if action is performed
      \retval false if action is failed or not needed.
    */
    virtual
    bool execute( PlayerAgent * agent ) = 0;

    /*!
      \brief create cloned action object
      \return pointer to the cloned object instance.
    */
    virtual
    ArmAction * clone() const = 0;
};

/////////////////////////////////////////////////////////////////////

/*!
  \class SoccerBehavior
  \brief abstract player behavior.
*/
class SoccerBehavior
    : public AbstractAction {
private:
    //! not used
    SoccerBehavior( const SoccerBehavior & );
    //! not used
    SoccerBehavior & operator=( const SoccerBehavior & );

protected
/*!
  \brief nothing to do. but accessible only from derived
  classes.
*/:
    SoccerBehavior()
      { }

public:
    /*!
      \brief nothing to do, but should be virtual.
    */
    virtual
    ~SoccerBehavior()
      { }

    /*!
      \brief pure virtual. set command to the action effector
      \retval true if action is performed
      \retval false if action is failed or not needed.
    */
    virtual
    bool execute( PlayerAgent * agent ) = 0;
};

}

#endif
