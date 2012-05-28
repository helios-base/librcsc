// -*-c++-*-

/*!
  \file player_object_updater.h
  \brief localizing and matching player objects using sensory data Header File
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

#ifndef RCSC_PLAYER_PLAYER_OBJECT_UPDATER_H
#define RCSC_PLAYER_PLAYER_OBJECT_UPDATER_H

#include <rcsc/player/localization.h>
#include <rcsc/player/player_object.h>

#include <list>

namespace rcsc {

class SelfObject;
class VisualSensor;

/*!
  \class PlayerObjectUpdater
  \brief player object
*/
class PlayerObjectUpdater {
private:

public:

    /*!
      \brief default constructor. initialize member variables

     */
    PlayerObjectUpdater()
      { }

    /*!
      \brief virtual destructor
     */
    virtual
    ~PlayerObjectUpdater()
      { }

    /*!
      \brief localize and matching seen players
      \param self agent information
      \param see visual sensor data
      \param localization localization algorithm
      \param old_teammate existing teammate object list
      \param old_opponent existing opponent object list
      \param old_opponent existing unknown player object list
     */
    virtual
    bool localizePlayers( const SelfObject & self,
                          const VisualSensor & see,
                          const Localization * localize,
                          PlayerObject::List & old_teammate,
                          PlayerObject::List & old_opponents,
                          PlayerObject::List & old_unknown_players );

protected:

    /*!
      \brief add new players using seen players
      \param side new players' team side
      \param seen_players seen players
      \param result_list result player list
     */
    void addNewPlayers( const SideID side,
                        const std::list< Localization::PlayerT > & seen_players,
                        PlayerObject::List & result_list );

};

}

#endif
