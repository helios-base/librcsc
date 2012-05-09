// -*-c++-*-

/*!
  \file action_types.h
  \brief player's action types Header File
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

#ifndef RCSC_PLAYER_ACTION_TYPES_H
#define RCSC_PLAYER_ACTION_TYPES_H

namespace rcsc {

/*!
  \enum ActionType
  \brief classifed action type ID
 */
enum ActionType {
    ACT_Shoot,
    ACT_Pass,
    ACT_Cross,
    ACT_Dribble,
    ACT_DribbleAvoid,
    ACT_Clear,
    ACT_Hold,

    ACT_GoToOpen,
    ACT_GoToForward,
    ACT_GoToOnside,
    ACT_GoToHome,

    ACT_GetBall,
    ACT_Mark,
    ACT_ShootBlock,
    ACT_PassBlock,
    ACT_DribbleBlock,

    ACT_StayThere,
    ACT_Recover,

    ACT_NoAction,

    ACT_Error,
};

}

#endif
