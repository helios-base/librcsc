// -*-c++-*-

/*!
  \file audio_memory.h
  \brief communication message data holder Header File
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

#ifndef RCSC_PLAYER_AUDIO_MEMORY_H
#define RCSC_PLAYER_AUDIO_MEMORY_H

#include <rcsc/geom/vector_2d.h>
#include <rcsc/game_time.h>

#include <iostream>
#include <string>
#include <vector>
#include <deque>

namespace rcsc {

/*!
  \class AudioMemory
  \brief the memory of heard data
*/
class AudioMemory {
public:

    /*!
      \struct Ball
      \brief heard ball info
     */
    struct Ball {
        int sender_; //!< message sender number
        Vector2D pos_; //!< heard ball position
        Vector2D vel_; //!< heard ball velocity

        /*!
          \brief initialize all member
         */
        Ball( const int sender,
              const Vector2D & pos,
              const Vector2D & vel )
            : sender_( sender ),
              pos_( pos ),
              vel_( vel )
          { }
    };

    /*!
      \struct Pass
      \brief heard pass info
     */
    struct Pass {
        int sender_; //!< message sender number
        int receiver_; //!< heard pass receiver number
        Vector2D receive_pos_; //!< heard pass receive pos

        /*!
          \brief initialize all member
         */
        Pass( const int sender,
              const int receiver,
              const Vector2D & pos )
            : sender_( sender ),
              receiver_( receiver ),
              receive_pos_( pos )
          { }
    };

    /*!
      \struct OurIntercept
      \brief heard our intercept info
     */
    struct OurIntercept {
        int sender_; //!< message sender number
        int interceptor_; //!< interceptor number
        int cycle_; //!< intercept cycle

        /*!
          \brief initialize all member
         */
        OurIntercept( const int sender,
                      const int interceptor,
                      const int cycle )
            : sender_( sender ),
              interceptor_( interceptor ),
              cycle_( cycle )
          { }
    };

    /*!
      \struct OppIntercept
      \brief heard opp intercept info
     */
    struct OppIntercept {
        int sender_; //!< message sender number
        int interceptor_; //!< interceptor number
        int cycle_; //!< intercept cycle

        /*!
          \brief initialize all member
         */
        OppIntercept( const int sender,
                      const int interceptor,
                      const int cycle )
            : sender_( sender )
            , interceptor_( interceptor )
            , cycle_( cycle )
          { }
    };

    /*!
      \struct Goalie
      \brief opponent goalie info
     */
    struct Goalie {
        int sender_; //!< goalie message sender number
        Vector2D pos_; //!< heard goalie positon
        AngleDeg body_; //!< heard goalie's body angle

        /*!
          \brief initialize all member
        */
        Goalie( const int sender,
                const Vector2D & pos,
                const AngleDeg & body )
            : sender_( sender ),
              pos_( pos ),
              body_( body )
          { }
    };

    /*!
      \struct Player
      \brief player info
     */
    struct Player {
        int sender_; //!< player message sender number
        int unum_; //!< heard player unum. if opponent player, += 11
        Vector2D pos_; //!< heard player position
        double body_; //!< heard player's body angle
        double stamina_; //!< heard player's stamina info. negative value means stamina is unknown

        /*!
          \brief initialize all member
        */
        Player( const int sender,
                const int unum,
                const Vector2D & pos,
                const double & body = -360.0,
                const double & stamina = -1.0 )
            : sender_( sender ),
              unum_( unum ),
              pos_( pos ),
              body_( body ),
              stamina_( stamina )
          { }
    };

    /*!
      \struct OffsideLine
      \brief offside line info
     */
    struct OffsideLine {
        int sender_; //!< offside line inf sender number
        double x_; //!< heard offside line value

        /*!
          \brief initialize all member
        */
        OffsideLine( const int sender,
                     const double & x )
            : sender_( sender ),
              x_( x )
          { }
    };

    /*!
      \struct DefenseLine
      \brief defense line info
     */
    struct DefenseLine {
        int sender_; //!< defense line message sender number
        double x_; //!< heard defense line value

        /*!
          \brief initialize all member
        */
       DefenseLine( const int sender,
                    const double & x )
            : sender_( sender ),
              x_( x )
          { }
    };

    /*!
      \struct WaitRequest
      \brief wait request info
     */
    struct WaitRequest {
        int sender_; //!< wait request message sender number;

        /*!
          \brief initialize all member
        */
        WaitRequest( const int sender )
            : sender_( sender )
          { }
    };

    /*!
      \struct Setplay
      \brief setplay info
     */
    struct Setplay {
        int sender_; //!< wait request message sender number;
        int wait_step_;

        /*!
          \brief initialize all member
        */
        Setplay( const int sender,
                 const int wait_step )
            : sender_( sender ),
              wait_step_( wait_step )
          { }
    };

    /*!
      \struct PassRequest
      \brief pass request info
     */
    struct PassRequest {
        int sender_; //!< pass request sendeer number
        Vector2D pos_; //!< heard pass request position

        /*!
          \brief initialize all member
        */
        PassRequest( const int sender,
                     const Vector2D & pos )
            : sender_( sender ),
              pos_( pos )
          { }
    };

    /*!
      \struct RunRequest
      \brief run request info
     */
    struct RunRequest {
        int sender_; //!< run request sendeer number
        int runner_; //!< heard runner number
        Vector2D pos_; //!< heard run request position

        /*!
          \brief initialize all member
        */
        RunRequest( const int sender,
                    const int runner,
                    const Vector2D & pos )
            : sender_( sender ),
              runner_( runner ),
              pos_( pos )
          { }
    };

    /*!
      \struct Stamina
      \brief stamina info
     */
    struct Stamina {
        int sender_; //!< stamina sender number
        double rate_; //!< heard stamina rate

        /*!
          \brief initialize all member
        */
        Stamina( const int sender,
                 const double & rate )
            : sender_( sender ),
              rate_( rate )
          { }
    };

    /*!
      \param struct Recovery
      \brief recovery info
     */
    struct Recovery {
        int sender_; //!< recovery sender number
        double rate_; //!< heard recovery rate

        /*!
          \brief initialize all member
        */
        Recovery( const int sender,
                  const double & rate )
            : sender_( sender ),
              rate_( rate )
          { }
    };

    /*!
      \struct StaminaCapacity
      \brief stamina capacity data
     */
    struct StaminaCapacity {
        int sender_; //!< stamina sender number
        double rate_; //!< heard stamina rate

        /*!
          \brief initialize all member
        */
        StaminaCapacity( const int sender,
                         const double & rate )
            : sender_( sender ),
              rate_( rate )
          { }
    };

    /*!
      \struct Dribble
      \brief dribble info
     */
    struct Dribble {
        int sender_; //!< heard dribble info sendeer number
        Vector2D target_; //!< heard dribble target point
        int queue_count_; //!< the size of dribble action queue

        /*!
          \brief initialize all member
        */
        Dribble( const int sender,
                 const Vector2D & target,
                 const int queue_count )
            : sender_( sender ),
              target_( target ),
              queue_count_( queue_count )
          { }
    };

    /*!
      \brief FreeMessage
      \brief free message holder
     */
    struct FreeMessage {
        int sender_; //!< freeform message
        std::string message_; //!< freeform message

        /*!
          \brief initialize all member
        */
        FreeMessage( const int sender,
                     const std::string & message )
            : sender_( sender ),
              message_( message )
          { }
    };


    //! alias of player record container type
    typedef std::deque< std::pair< GameTime, Player > > PlayerRecord;

protected:

    //! last updated time
    GameTime M_time;

    std::vector< Ball > M_ball; //!< heard info
    GameTime M_ball_time; //!< heard time

    std::vector< Pass > M_pass; //!< heard info
    GameTime M_pass_time; //!< heard time

    std::vector< OurIntercept > M_our_intercept; //!< heard info
    GameTime M_our_intercept_time; //!< heard time

    std::vector< OppIntercept > M_opp_intercept; //!< heard info
    GameTime M_opp_intercept_time; //!< heard time

    std::vector< Goalie > M_goalie; //!< heard info
    GameTime M_goalie_time; //!< heard time

    std::vector< Player > M_player; //!< heard info
    GameTime M_player_time; //!< heard time

    std::vector< OffsideLine > M_offside_line; //!< heard info
    GameTime M_offside_line_time; //!< heard time

    std::vector< DefenseLine > M_defense_line; //!< heard info
    GameTime M_defense_line_time; //!< heard time

    std::vector< WaitRequest > M_wait_request; //!< heard info
    GameTime M_wait_request_time; //!< heard time

    std::vector< Setplay > M_setplay; //!< heard info
    GameTime M_setplay_time; //!< heard time

    std::vector< PassRequest > M_pass_request; //!< heard info
    GameTime M_pass_request_time; //!< heard time

    std::vector< RunRequest > M_run_request; //!< heard info
    GameTime M_run_request_time; //!< heard time

    std::vector< Stamina > M_stamina; //!< heard info
    GameTime M_stamina_time; //!< heard time

    std::vector< Recovery > M_recovery; //!< heard info
    GameTime M_recovery_time; //!< heard time

    std::vector< StaminaCapacity > M_stamina_capacity; //!< heard info
    GameTime M_stamina_capacity_time; //!< heard time

    std::vector< Dribble > M_dribble; //!< heard info
    GameTime M_dribble_time; //!< heard time

    std::vector< FreeMessage > M_free_message; //!< heard info
    GameTime M_free_message_time; //!< heard time


    //! memory of heared players
    PlayerRecord M_player_record;


private:
    // not used
    AudioMemory( const AudioMemory & ) = delete;
    AudioMemory & operator=( const AudioMemory & ) = delete;

public:

    /*!
      \brief initialize member variables
    */
    AudioMemory();

    /*!
      \brief virtual destructor
    */
    virtual
    ~AudioMemory() = default;

    // accessor methods

    const GameTime & time() const
      {
          return M_time;
      }

    /*!
      \brief get heard ball info
      \return ball info container
    */
    const std::vector< Ball > & ball() const
      {
          return M_ball;
      }

    /*!
      \brief ball info heard time
      \return time value
    */
    const GameTime & ballTime() const
      {
          return M_ball_time;
      }

    /*!
      \brief get heard pass info
      \return pass info container
    */
    const std::vector< Pass > & pass() const
      {
          return M_pass;
      }

    /*!
      \brief get pas info heard time
      \return time value
    */
    const GameTime & passTime() const
      {
          return M_pass_time;
      }

    /*!
      \brief get heard our interceptor info
      \return our intercept info container
    */
    const std::vector< OurIntercept > & ourIntercept() const
      {
          return M_our_intercept;
      }

    /*!
      \brief get our intercept info heard time
      \return time value
    */
    const GameTime & ourInterceptTime() const
      {
          return M_our_intercept_time;
      }

    /*!
      \brief get opp intercept info
      \return opp intercept info container
    */
    const std::vector< OppIntercept > & oppIntercept() const
      {
          return M_opp_intercept;
      }

    /*!
      \brief get opp intercept info heard time
      \return time value
    */
    const GameTime & oppInterceptTime() const
      {
          return M_opp_intercept_time;
      }

    /*!
      \brief get heard goalie info
      \return goalie info container
    */
    const std::vector< Goalie > & goalie() const
      {
          return M_goalie;
      }

    /*!
      \brief get goalie info heard time
      \return time value
    */
    const GameTime & goalieTime() const
      {
          return M_goalie_time;
      }

    /*!
      \brief get heard player
      \return player info container
    */
    const std::vector< Player > & player() const
      {
          return M_player;
      }

    /*!
      \brief get player info heard time
      \return time value
    */
    const GameTime & playerTime() const
      {
          return M_player_time;
      }

    /*!
      \brief get player record container
      \return PlayerRecord
     */
    const PlayerRecord & playerRecord() const { return M_player_record; }

    /*!
      \brief get heard offside line info
      \return offside line info container
    */
    const std::vector< OffsideLine > & offsideLine() const
      {
          return M_offside_line;
      }

    /*!
      \brief get offside line heard time
      \return time value
    */
    const GameTime & offsideLineTime() const
      {
          return M_offside_line_time;
      }

    /*!
      \brief get heard defense line
      \return defense line info container
    */
    const std::vector< DefenseLine > & defenseLine() const
      {
          return M_defense_line;
      }

    /*!
      \brief get defense line heard time
      \return time value
    */
    const GameTime & defenseLineTime() const
      {
          return M_defense_line_time;
      }

    /*!
      \brief get wait request info
      \return wait request info container
    */
    const std::vector< WaitRequest > & waitRequest() const
      {
          return M_wait_request;
      }

    /*!
      \brief get wait request heard time
      \return time value
    */
    const GameTime & waitRequestTime() const
      {
          return M_wait_request_time;
      }

    /*!
      \brief get setplay info
      \return setplay info container
     */
    const std::vector< Setplay > & setplay() const
      {
          return M_setplay;
      }

    /*!
      \brief get setplay heared time
      \return time value
     */
    const GameTime & setplayTime() const
      {
          return M_setplay_time;
      }

    /*!
      \brief get pass request info
      \return pass request info container
    */
    const std::vector< PassRequest > & passRequest() const
      {
          return M_pass_request;
      }

    /*!
      \brief get pass request heard time
      \return time value
    */
    const GameTime & passRequestTime() const
      {
          return M_pass_request_time;
      }

    /*!
      \brief get run request info
      \return run request info container
    */
    const std::vector< RunRequest > & runRequest() const
      {
          return M_run_request;
      }

    /*!
      \brief get run request heard time
      \return time value
    */
    const GameTime & runRequestTime() const
      {
          return M_run_request_time;
      }

    /*!
      \brief get heard stamina info
      \return stamina info container
    */
    const std::vector< Stamina > & stamina() const
      {
          return M_stamina;
      }

    /*!
      \brief get stamina info heard time
      \return time value
    */
    const GameTime & staminaTime() const
      {
          return M_stamina_time;
      }

    /*!
      \brief get heard recovery info
      \return recovery info container
    */
    const std::vector< Recovery > & recovery() const
      {
          return M_recovery;
      }

    /*!
      \brief get recovery info heard time
      \return time value
    */
    const GameTime & recoveryTime() const
      {
          return M_recovery_time;
      }

    /*!
      \brief get heard stamina capacity info
      \return stamina capacity info container
    */
    const std::vector< StaminaCapacity > & staminaCapacity() const
      {
          return M_stamina_capacity;
      }

    /*!
      \brief get stamina info heard time
      \return time value
    */
    const GameTime & staminaCapacityTime() const
      {
          return M_stamina_capacity_time;
      }

    /*!
      \brief get dribble info
      \return dribble info container
    */
    const std::vector< Dribble > & dribble() const
      {
          return M_dribble;
      }

    /*!
      \brief get dribble info heard time
      \return time value
    */
    const GameTime & dribbleTime() const
      {
          return M_dribble_time;
      }

    /*!
      \brief get free message
      \return free message container
     */
    const std::vector< FreeMessage > & freeMessage() const
      {
          return M_free_message;
      }

    /*!
      \brief get free message heard time
      \return time value
     */
    const GameTime & freeMessageTime() const
      {
          return M_free_message_time;
      }


    // setter methods

    /*!
      \brief set heard ball status
      \param sender sender's uniform number
      \param pos heard ball posision
      \param vel heard ball velocity
      \param current current game time
    */
    virtual
    void setBall( const int sender,
                  const Vector2D & pos,
                  const Vector2D & vel,
                  const GameTime & current );

    /*!
      \brief set heard pass info
      \param sender sender's uniform number
      \param receiver receiver number
      \param pos pass receive point
      \param current current game time
    */
    virtual
    void setPass( const int sender,
                  const int receiver,
                  const Vector2D & pos,
                  const GameTime & current );

    /*!
      \brief set heard pass request info
      \param sender sender's uniform number
      \param interceptor interceptor's number[1,22]. 11+ means opponent
      \param cycle estimated intercept cycle
      \param current current game time
    */
    virtual
    void setIntercept( const int sender,
                       const int interceptor,
                       const int cycle,
                       const GameTime & current );

    /*!
      \brief set heard opponent goalie info
      \param sender sender's uniform number
      \param pos heard opponent goalie pos
      \param body heard opponent goalie's body angle
      \param current current game time
    */
    virtual
    void setOpponentGoalie( const int sender,
                            const Vector2D & pos,
                            const AngleDeg & body,
                            const GameTime & current );

    /*!
      \brief set heard player info
      \param sender sender's uniform number
      \param unum heard player unum
      \param pos heard player pos
      \param current current game time
    */
    virtual
    void setPlayer( const int sender,
                    const int unum,
                    const Vector2D & pos,
                    const GameTime & current );

    /*!
      \brief set heard player info
      \param sender sender's uniform number
      \param unum heard player unum
      \param pos heard player pos
      \param body heard player body angle
      \param stamina heard player's stamina. negative value means stamina is unknown.
      \param current current game time
    */
    virtual
    void setPlayer( const int sender,
                    const int unum,
                    const Vector2D & pos,
                    const double & body,
                    const double & stamina,
                    const GameTime & current );

    /*!
      \brief set heard offside line info
      \param sender sender's uniform number
      \param offside_line_x heard value
      \param current current game time
    */
    virtual
    void setOffsideLine( const int sender,
                         const double & offside_line_x,
                         const GameTime & current );

    /*!
      \brief set heard defense line info
      \param sender sender's uniform number
      \param defense_line_x heard value
      \param current current game time
    */
    virtual
    void setDefenseLine( const int sender,
                         const double & defense_line_x,
                         const GameTime & current );

    /*!
      \brief set heard wait request info
      \param sender sender's uniform number
      \param current current game time
    */
    virtual
    void setWaitRequest( const int sender,
                         const GameTime & current );

    /*!
      \brief set heard setplay info
      \param sender sender's uniform number
      \param wait_step time step until the setplay will start
      \param current current game time
     */
    virtual
    void setSetplay( const int sender,
                     const int wait_step,
                     const GameTime & current );

    /*!
      \brief set heard pass request info
      \param sender message sender's uniform number
      \param request_pos request position
      \param current current game time
    */
    virtual
    void setPassRequest( const int sender,
                         const Vector2D & request_pos,
                         const GameTime & current );

    /*!
      \brief set heard run request info
      \param sender message sender's uniform number
      \param runner runner's uniform number
      \param request_pos request position
      \param current current game time
    */
    virtual
    void setRunRequest( const int sender,
                        const int runner,
                        const Vector2D & request_pos,
                        const GameTime & current );

    /*!
      \brief set heard stamina info
      \param sender message sender's uniform number
      \param rate stamina value rate
      \param current current game time
    */
    virtual
    void setStamina( const int sender,
                     const double & rate,
                     const GameTime & current );

    /*!
      \brief set heard recovery info
      \param sender message sender's uniform number
      \param rate recovery value rate
      \param current current game time
    */
    virtual
    void setRecovery( const int sender,
                      const double & rate,
                      const GameTime & current );

    /*!
      \brief set heard stamina capacity info
      \param sender message sender's uniform number
      \param rate stamina value rate
      \param current current game time
    */
    virtual
    void setStaminaCapacity( const int sender,
                             const double & rate,
                             const GameTime & current );

    /*!
      \brief set heard dribble target point
      \param sender message sender's uniform number
      \param pos target position
      \param queue_count dribble queue count
      \param current current game time
    */
    virtual
    void setDribbleTarget( const int sender,
                           const Vector2D & pos,
                           const int queue_count,
                           const GameTime & current );

    /*!
      \brief set heard freeform message
      \param sender message sender's uniform number
      \param msg heard message
      \param current current game time
     */
    virtual
    void setFreeMessage( const int sender,
                         const std::string & msg,
                         const GameTime & current );

    virtual
    std::ostream & printDebug( std::ostream & os ) const;
};

}

#endif
