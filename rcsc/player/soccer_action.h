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

#include <memory>

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
    AbstractAction( const AbstractAction & ) = delete;
    //! not used
    AbstractAction & operator=( const AbstractAction & ) = delete;

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
    ~AbstractAction() = default;

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
    BodyAction( const BodyAction & ) = delete;
    //! not used
    BodyAction & operator=( const BodyAction & ) = delete;

protected:
    /*!
      \brief nothing to do. but accessible only from derived
      classes.
    */
    BodyAction() = default;

public:
    /*!
      \brief nothing to do, but should be virtual.
    */
    virtual
    ~BodyAction() = default;

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
    typedef std::shared_ptr< NeckAction > Ptr;

private:

    //! not used
    NeckAction( const NeckAction & ) = delete;
    //! not used
    NeckAction & operator=( const NeckAction & ) = delete;

protected:
    /*!
      \brief nothing to do. but accessible only from derived
      classes.
    */
    NeckAction() = default;

public:
    /*!
      \brief nothing to do, but should be virtual.
    */
    virtual
    ~NeckAction() = default;

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
    typedef std::shared_ptr< ViewAction > Ptr;

private:

    //! not used
    ViewAction( const ViewAction & ) = delete;
    //! not used
    ViewAction & operator=( const ViewAction & ) = delete;

protected:
    /*!
      \brief nothing to do. but accessible only from derived
      classes.
    */
    ViewAction() = default;

public:
    /*!
      \brief nothing to do, but should be virtual.
    */
    virtual
    ~ViewAction() = default;

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
  \class FocusAction
  \brief abstract change_focus action
 */
class FocusAction
    : public AbstractAction {
public:

    typedef std::shared_ptr< FocusAction > Ptr;
private:

    //! not used
    FocusAction( const FocusAction & ) = delete;
    //! not used
    FocusAction & operator=( const FocusAction & ) = delete;

protected:

    /*!
      \brief nothing to do. but accessible only from derived classes.
    */
    FocusAction() = default;

public:
    /*!
      \brief nothing to do, but should be a virtual method.
    */
    virtual
    ~FocusAction() = default;

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
    FocusAction * clone() const = 0;

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
    typedef std::shared_ptr< ArmAction > Ptr;

private:

    //! not used
    ArmAction( const ArmAction & ) = delete;
    //! not used
    ArmAction & operator=( const ArmAction & ) = delete;

protected:
    /*!
      \brief nothing to do. but accessible only from derived
      classes.
    */
    ArmAction() = default;

public:
    /*!
      \brief nothing to do, but should be virtual.
    */
    virtual
    ~ArmAction() = default;

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
    SoccerBehavior( const SoccerBehavior & ) = delete;
    //! not used
    SoccerBehavior & operator=( const SoccerBehavior & ) = delete;

protected:

    /*!
      \brief nothing to do. but accessible only from derived
      classes.
    */
    SoccerBehavior() = default;

public:
    /*!
      \brief nothing to do, but should be virtual.
    */
    virtual
    ~SoccerBehavior() = default;

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
