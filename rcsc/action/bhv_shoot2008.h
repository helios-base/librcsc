// -*-c++-*-

/*!
  \file bhv_shoot2008.h
  \brief advanced shoot planning and behavior.
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

#ifndef RCSC_ACTION_BHV_SHOOT2008_H
#define RCSC_ACTION_BHV_SHOOT2008_H

#include <rcsc/action/shoot_table2008.h>

#include <rcsc/player/soccer_action.h>

namespace rcsc {

/*!
  \class Bhv_Shoot2008
  \brief advanced shoot planning and behavior.
 */
class Bhv_Shoot2008
    : public SoccerBehavior {
private:
    //! cached shoot table
    static ShootTable2008 S_shoot_table;

public:
    /*!
      \brief accessible from global.
     */
    Bhv_Shoot2008()
      { }

    /*!
      \brief execute action
      \param agent pointer to the agent itself
      \return true if action is performed
     */
    bool execute( PlayerAgent * agent );


    /*!
      \brief get the current shoot table
      \return const reference to the shoot table instance.
     */
    static
    ShootTable2008 & shoot_table()
      {
          return S_shoot_table;
      }

};

}

#endif
