// -*-c++-*-

/*!
  \file soccer_intention.h
  \brief abstract player intention Header File
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

#ifndef RCSC_PLAYER_SOCCER_INTENTION_H
#define RCSC_PLAYER_SOCCER_INTENTION_H

#include <memory>
#include <string>
#include <queue>

namespace rcsc {

class PlayerAgent;

/*!
  \class SoccerIntention
  \brief abstrace player agent's intention class
*/
class SoccerIntention {
public:

    //! smart pointer type.
    typedef std::shared_ptr< SoccerIntention > Ptr;

private:
    //! not used
    SoccerIntention( const SoccerIntention & ) = delete;
    //! not used
    SoccerIntention & operator=( const SoccerIntention & ) = delete;

protected:
    /*!
      \brief constructor is used only from derived class.
     */
    SoccerIntention() = default;

public:
    /*!
      \brief virtual destructor
     */
    virtual
    ~SoccerIntention() = default;

    ///////////////////////////////////////////////

    // virtual functions

    /*!
      \brief (pure virtual) check if this intention is finished or not
      \return true if intention is finished
     */
    virtual
    bool finished( const PlayerAgent * agent ) = 0;

    /*!
      \brief execute suitable action
      \return result of action
     */
    virtual
    bool execute( PlayerAgent * agent ) = 0;
};

/////////////////////////////////////////////////////////////

/*!
  \class SoccerIntentionQueue
  \brief intention queue
*/
class SoccerIntentionQueue
    : public SoccerIntention {
private:

    //! intention queue
    std::queue< Ptr >  M_internal_q;

public:

    /*!
      \brief check if this intention queue is finished or not
      \return true if intention is finished
     */
    bool finished( const PlayerAgent * agent )
      {
          while ( ! M_internal_q.empty()
                  && M_internal_q.front()->finished( agent ) )
          {
              M_internal_q.pop();
          }

          if ( M_internal_q.empty() )
          {
              return true;
          }
          return false;
      }

    /*!
      \brief execute queued intention
      \return result of action
     */
    bool execute( PlayerAgent * agent )
      {
          return pop( agent );
      }

    /*!
      \brief push new intention
      \param intention dynamic allocated intention pointer
     */
    void push( Ptr intention )
      {
          M_internal_q.push( intention );
      }

    /*!
      \brief execute queued intention.
      \param agent agent object pointer
      \retval true if action is executed
      \retval false if queue is empty, or action is failed.
     */
    bool pop( PlayerAgent * agent )
      {
          while ( ! M_internal_q.empty()
                  && M_internal_q.front()->finished( agent ) )
          {
              M_internal_q.pop();
          }

          if ( M_internal_q.empty() )
          {
              return false;
          }

          return M_internal_q.front()->execute( agent );
      }

    /*!
      \brief clear queued intention.
     */
    void clear()
      {
          while ( ! M_internal_q.empty() )
          {
              M_internal_q.pop();
          }
      }
};

}

#endif
