// -*-c++-*-

/*!
  \file coach_world_model.cpp
  \brief coach/trainer world model class Source File
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "coach_world_model.h"

#include "coach_visual_sensor.h"

#include <rcsc/clang/clang_message.h>
#include <rcsc/common/server_param.h>
#include <rcsc/common/player_param.h>
#include <rcsc/common/player_type.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/audio_memory.h>
#include <rcsc/geom/rect_2d.h>

#include <iostream>
#include <cstdio>

namespace rcsc {

namespace {
const char * clang_type_names[] = CLANG_TYPE_NAMES;
}

/*-------------------------------------------------------------------*/
/*!

 */
CoachWorldModel::CoachWorldModel()
    : M_client_version( 0 ),
      M_time( -1, 0 ),
      M_see_time( -1, 0 ),
      M_our_side( NEUTRAL ),
      M_last_set_play_start_time( 0, 0 ),
      M_setplay_count( 0 ),
      M_game_mode(),
      M_training_time( -1, 0 ),
      M_audio_memory( new AudioMemory() ),
      M_current_state( new CoachWorldState() ),
      M_last_kicker_side( NEUTRAL ),
      M_last_kicker_unum( Unum_Unknown ),
      M_pass_time( -1, 0 ),
      M_passer_unum( Unum_Unknown ),
      M_receiver_unum( Unum_Unknown ),
      M_pass_start_pos( Vector2D::INVALIDATED ),
      M_pass_receive_pos( Vector2D::INVALIDATED ),
      M_player_type_analyzer( *this ),
      M_our_substitute_count( 0 ),
      M_their_substitute_count( 0 ),
      M_last_playon_start( 0 ),
      M_clang_capacity_update_time( -1 ),
      M_freeform_allowed_count( ServerParam::i().coachSayCountMax() ),
      M_freeform_send_count( 0 )
{
    for ( int i = 0; i < 11; i++ )
    {
        M_our_player_type_id[i] = Hetero_Default;
        M_their_player_type_id[i] = Hetero_Default;
        M_our_card[i] = NO_CARD;
        M_their_card[i] = NO_CARD;
    }

    M_our_player_type_used_count.push_back( 11 );
    M_their_player_type_used_count.push_back( 11 );

    for ( int i = 0; i < CLANG_MAX_TYPE; ++i )
    {
        M_clang_capacity[i] = 0;
    }
    M_clang_capacity[CLANG_UNSUPP] = 1;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldModel::init( const std::string & team_name,
                       const SideID side,
                       const int client_version )
{
    M_our_team_name = team_name;
    M_our_side = side;
    M_client_version = client_version;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldModel::setAudioMemory( std::shared_ptr< AudioMemory > memory )
{
    M_audio_memory = memory;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldModel::setServerParam()
{
    M_freeform_allowed_count = ServerParam::i().coachSayCountMax();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldModel::setPlayerParam()
{
    const int player_types = PlayerParam::i().playerTypes();
    const int pt_max = PlayerParam::i().ptMax();

    //
    // default type
    //
    if ( ! PlayerParam::i().allowMultDefaultType() )
    {
        for ( int i = 0; i < pt_max; ++i )
        {
            M_available_player_type_id.push_back( Hetero_Default );
        }
    }
    else
    {
        M_available_player_type_id.push_back( Hetero_Default );
    }

    //
    // other types
    //
    for ( int id = 1; id < player_types; ++id )
    {
        for ( int i = 0; i < pt_max; ++i )
        {
            M_available_player_type_id.push_back( id );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldModel::setTeamName( const SideID side,
                              const std::string & name )
{
    if ( name.empty() )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " *** ERROR *** Empty team name "
                  << std::endl;
    }

    if ( side == ourSide()
         || ( ourSide() == NEUTRAL
              && side == LEFT ) )
    {
        M_our_team_name = name;
    }
    else if ( side != ourSide() )
    {
        M_their_team_name = name;
    }
    else
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " *** ERROR *** Invalid side = " << side
                  << std::endl;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldModel::changePlayerType( const SideID side,
                                   const int unum,
                                   const int type )
{
    dlog.addText( Logger::WORLD,
                  __FILE__":(changePlayerType) side=%c unum=%d type=%d",
                  side_char( side ), unum, type );

    if ( side == NEUTRAL
         || unum < 1 || 11 < unum )
    {
        std::cerr << ourTeamName() << " coach: " << time()
                  << " ***ERROR*** (changePlayerType) Illegal player id."
                  << " side=" << side_str( side )
                  << " unum=" << unum << std::endl;
        return;
    }

    const int player_types = PlayerParam::i().playerTypes();

    if ( type != Hetero_Unknown
         && ( type < Hetero_Default || player_types <= type ) )
    {
        std::cerr << ourTeamName() << " coach: " << time()
                  << " ***ERROR*** (changePlayerType) Illegal player type id "
                  << type << std::endl;
        return;
    }

    if ( ourSide() == side
         || ( ourSide() == NEUTRAL
              && side == LEFT ) )
    {
        M_our_player_type_id[unum - 1] = type;

        if ( this->time().cycle() > 0 )
        {
            ++M_our_substitute_count;
        }

        M_our_player_type_used_count.assign( player_types, 0 );
        for ( int i = 0; i < 11; ++i )
        {
            const int t = M_our_player_type_id[i];
            if ( t != Hetero_Unknown )
            {
                M_our_player_type_used_count[t] += 1;
            }
        }

        M_our_card[unum - 1] = NO_CARD;
    }
    else
    {
        M_their_player_type_id[unum - 1] = type;

        if ( this->time().cycle() > 0 )
        {
            ++M_their_substitute_count;
        }

        M_their_player_type_used_count.assign( player_types, 0 );
        for ( int i = 0; i < 11; ++i )
        {
            const int t = M_their_player_type_id[i];
            if ( t != Hetero_Unknown )
            {
                M_their_player_type_used_count[t] += 1;
            }
        }

        M_their_card[unum - 1] = NO_CARD;
    }

    //
    // if the player is a teammate, erase that type from available types.
    //
    if ( ourSide() == side )
    {
        if ( type == Hetero_Default
             && PlayerParam::i().allowMultDefaultType() )
        {
            // nothing to do
        }
        else
        {
            std::vector< int >::iterator it = std::find( M_available_player_type_id.begin(),
                                                         M_available_player_type_id.end(),
                                                         type );
            if ( it != M_available_player_type_id.end() )
            {
                dlog.addText( Logger::WORLD,
                              __FILE__": erase available player type %d", *it );
                M_available_player_type_id.erase( it );
            }
        }
    }

    //
    // if the player is their team's player, reset analyzed result.
    //
    if ( side != ourSide()
         && type == Hetero_Unknown )
    {
        M_player_type_analyzer.reset( unum );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldModel::setCard( const SideID side,
                          const int unum,
                          const Card card )
{
    if ( side == NEUTRAL
         || unum < 1 || 11 < unum )
    {
        std::cerr << ourTeamName() << " coach:"
                  << " ***ERROR*** (CoachWorldModel::setCard) "
                  << " Illegal player id. side=" << side
                  << " unum=" << unum << std::endl;
        return;
    }

    M_current_state->setCard( side, unum, card );

    if ( ourSide() == side
         || ( ourSide() == NEUTRAL
              && side == LEFT ) )
    {
        M_our_card[unum - 1] = card;

    }
    else
    {
        M_their_card[unum - 1] = card;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldModel::updateGameMode( const GameMode & game_mode,
                                 const GameTime & current )
{
    bool pk_mode = game_mode.isPenaltyKickMode();

    if ( ! pk_mode
         && game_mode.type() != GameMode::PlayOn )
    {
        if ( gameMode().type() != game_mode.type() )
        {
            M_last_set_play_start_time = current;
            M_setplay_count = 0;
        }

        // check human referee's interaction
        if ( gameMode().type() == game_mode.type()
             && game_mode.type() == GameMode::FreeKick_ )
        {
            M_last_set_play_start_time = current;
            M_setplay_count = 0;
        }
    }

    if ( gameMode().type() != GameMode::PlayOn
         && game_mode.type() == GameMode::PlayOn )
    {
        M_last_playon_start = current.cycle();
    }

    M_time = current;
    M_game_mode = game_mode;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldModel::updateAfterSeeGlobal( const CoachVisualSensor & see_global,
                                       const GameTime & current )
{
    dlog.addText( Logger::WORLD,
                  __FILE__": (updateAfterSeeGlobal)" );

    if ( M_see_time.cycle() != current.cycle() - 1
         && M_see_time.stopped() != current.stopped() - 1 )
    {
        // missed cycles??
        if ( M_see_time.cycle() == current.cycle()
             && M_see_time.stopped() > 0
             && current.stopped() == 0 )
        {
            // back from the stopped mode
        }
        else
        {
            std::cerr << __FILE__ << ' ' << __LINE__
                      << " missed cycles? last updated time = " << M_see_time
                      << " current = " << current
                      << std::endl;
        }
    }

    M_time = current;

    if ( M_see_time == current )
    {
        return;
    }
    M_see_time = current;

    updateTeamNames( see_global );

    M_previous_state = M_current_state;
    M_current_state = CoachWorldState::Ptr( new CoachWorldState( see_global,
                                                                 ourSide(),
                                                                 current,
                                                                 M_game_mode,
                                                                 M_previous_state ) );
    updatePlayerType();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldModel::updateJustBeforeDecision( const GameTime & current )
{
    M_time = current;

    ++M_setplay_count;

    updateCLangCapacity();

    updateLastKicker();
    updateLastPasser();

    M_current_state->updatePlayerStamina( *M_audio_memory );

    //
    // store the latest state data
    //
    if ( gameMode().type() == GameMode::BeforeKickOff
         || gameMode().type() == GameMode::TimeOver )
    {
        // if ( ! M_state_list.empty()
        //      && M_state_list.back()->gameMode().type() == gameMode().type() )
        // {
        //     M_state_map.erase( M_state_list.back()->time() );
        //     M_state_list.pop_back();
        // }
        // M_state_map[ current ] = M_current_state;
        // M_state_list.push_back( M_current_state );
    }
    else
    {
        M_state_list.push_back( M_current_state );
        M_state_map[ current ] = M_current_state;
    }

    //
    // erase the oldest state data
    //
    if ( M_state_list.size() > 60000 )
    {
        GameMode::Type mode = M_state_list.front()->gameMode().type();
        GameTime t = M_state_list.front()->time();
        if ( mode == GameMode::BeforeKickOff
             || mode == GameMode::TimeOver )
        {
            t.setStopped( 0 );
        }

        M_state_map.erase( t );
        M_state_list.pop_front();
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldModel::updateCLangCapacity()
{
    const ServerParam & SP = ServerParam::i();

    //
    // update clang message capacity
    //
    if ( M_clang_capacity_update_time < 0
         || M_time.cycle() - M_clang_capacity_update_time >= SP.clangWinSize() )
    {
        M_clang_capacity[CLANG_DEFINE] = SP.clangDefineWin();
        M_clang_capacity[CLANG_META] = SP.clangMetaWin();
        M_clang_capacity[CLANG_ADVICE] = SP.clangAdviceWin();
        M_clang_capacity[CLANG_INFO] = SP.clangInfoWin();
        M_clang_capacity[CLANG_DEL] = SP.clangDelWin();
        M_clang_capacity[CLANG_RULE] = SP.clangRuleWin();

        M_clang_capacity_update_time = M_time.cycle();
        if ( M_clang_capacity_update_time <= 1 )
        {
            M_clang_capacity_update_time = 1;
        }

        if ( M_time.stopped() == 0
             && ourSide() != NEUTRAL )
        {
            std::cerr << ourTeamName() << " coach: " << M_time
                      << " updated clang capacity." << std::endl;
        }
    }

    //
    // update freeform capacity
    //
    if ( M_time.cycle() > 0
         && M_time.stopped() == 0
         && SP.halfTime() > 0
         && SP.nrNormalHalfs() > 0
         && M_time.cycle() % ( SP.actualHalfTime() * SP.nrNormalHalfs() ) == 0 )
    {
        M_freeform_allowed_count += SP.coachSayCountMax();
        if ( ourSide() != NEUTRAL )
        {
            std::cerr << ourTeamName() << " coach: " << M_time
                      << " new freeform allowed " << M_freeform_allowed_count
                      << std::endl;
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldModel::updateTeamNames( const CoachVisualSensor & see_global )
{
    if ( ! see_global.teamNameLeft().empty() )
    {
        if ( ourSide() == RIGHT )
        {
            M_their_team_name = see_global.teamNameLeft();
        }
        else
        {
            M_our_team_name = see_global.teamNameLeft();
        }
    }

    if ( ! see_global.teamNameRight().empty() )
    {
        if ( ourSide() == RIGHT )
        {
            M_our_team_name = see_global.teamNameRight();
        }
        else
        {
            M_their_team_name = see_global.teamNameRight();
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldModel::updatePlayerType()
{
    if ( ourSide() == NEUTRAL )
    {
        return;
    }

    //
    // analyze
    //
    M_player_type_analyzer.update();

    //
    // update analyzed opponent player type id array
    //
    for ( int unum = 1; unum <= 11; ++unum )
    {
        int id = M_player_type_analyzer.playerTypeId( unum );
        if ( id != Hetero_Unknown )
        {
            M_their_player_type_id[unum-1] = id;
        }
    }

    //
    // update used count array
    //

    M_our_player_type_used_count.assign( PlayerParam::i().playerTypes(), 0 );
    for ( int i = 0; i < 11; ++i )
    {
        const int id = M_our_player_type_id[i];
        if ( id != Hetero_Unknown )
        {
            M_our_player_type_used_count[id] += 1;
        }
    }

    M_their_player_type_used_count.assign( PlayerParam::i().playerTypes(), 0 );
    for ( int i = 0; i < 11; ++i )
    {
        const int id = M_their_player_type_id[i];
        if ( id != Hetero_Unknown )
        {
            M_their_player_type_used_count[id] += 1;
        }
    }

    //
    // set to player object instance
    //

    for ( int i = 0; i < 11; ++i )
    {
        M_current_state->setTeammatePlayerType( i + 1, M_our_player_type_id[i] );
        M_current_state->setOpponentPlayerType( i + 1, M_their_player_type_id[i] );
    }
}

/*-------------------------------------------------------------------*/
/*!
  TODO: support old visual information that has no kick/tackle/foul information.
 */
void
CoachWorldModel::updateLastKicker()
{
    const CoachBallObject & cur_ball = M_current_state->ball();

    //
    // no playon
    //
    if ( gameMode().type() != GameMode::PlayOn )
    {
        if ( gameMode().isOurSetPlay( ourSide() ) )
        {
            M_last_kicker_side = ourSide();
        }
        else if ( gameMode().isTheirSetPlay( ourSide() ) )
        {
            M_last_kicker_side = theirSide();
        }
        else
        {
            M_last_kicker_side = NEUTRAL;
        }

        M_last_kicker_unum = Unum_Unknown;

        const CoachPlayerObject * p = getPlayerNearestTo( cur_ball.pos() );
        if ( p
             && p->side() == M_last_kicker_side )
        {
            M_last_kicker_unum = p->unum();
        }

        dlog.addText( Logger::WORLD,
                      __FILE__":(updateLastKicker) non-playon side=%d unum=%d",
                      M_last_kicker_side, M_last_kicker_unum );
        return;
    }

    if ( ! M_previous_state )
    {
        return;
    }

    const CoachPlayerObject * kicker = M_current_state->kicker();

    if ( M_current_state->kickerCandidates().size() > 1 )
    {
        M_last_kicker_side = NEUTRAL;
        M_last_kicker_unum = Unum_Unknown;
    }
    else if ( kicker )
    {
        M_last_kicker_side = kicker->side();
        M_last_kicker_unum = kicker->unum();
    }

    dlog.addText( Logger::WORLD,
                  __FILE__":(updateLastKicker) result side=%d unum=%d",
                  M_last_kicker_side, M_last_kicker_unum );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldModel::updateLastPasser()
{
    if ( ! M_previous_state )
    {
        return;
    }

    if ( M_audio_memory->pass().size() != 1 )
    {
        dlog.addText( Logger::WORLD,
                      __FILE__":(updateLastPasser) illegal pass size" );
        return;
    }

    if ( M_audio_memory->passTime() != M_current_state->time()
         && M_audio_memory->passTime() != M_previous_state->time() )
    {
        // no pass message
        dlog.addText( Logger::WORLD,
                      __FILE__":(updateLastPasser) time mismatch. pass_time=%ld",
                      M_audio_memory->passTime().cycle() );
        return;
    }

    const AudioMemory::Pass & pass = M_audio_memory->pass().front();

    if ( pass.sender_ < 1 || 11 < pass.sender_
         || pass.receiver_ < 1 || 11 < pass.receiver_ )
    {
        dlog.addText( Logger::WORLD,
                      __FILE__":(updateLastPasser) illegal passer or receiver." );
        return;
    }

    const CoachBallObject * ball = nullptr;
    const CoachPlayerObject * passer = nullptr;

    if ( M_audio_memory->passTime() == M_current_state->time() )
    {
        ball = &(M_current_state->ball());
        passer = M_current_state->teammate( pass.sender_ );
    }
    else if ( M_audio_memory->passTime() == M_previous_state->time() )
    {
        ball = &(M_previous_state->ball());
        passer = M_previous_state->teammate( pass.sender_ );
    }

    if ( ! passer )
    {
        dlog.addText( Logger::WORLD,
                      __FILE__":(updateLastPasser) No passer." );
        return;
    }

    // check if passer is really kickable in the previous cycle.
    const double kickable_area2 = std::pow( ( passer->playerTypePtr()
                                              ? passer->playerTypePtr()->kickableArea()
                                              : ServerParam::i().defaultKickableArea() ),
                                            2 );

    if ( passer->pos().dist2( ball->pos() ) > kickable_area2 )
    {
        dlog.addText( Logger::WORLD,
                      __FILE__":(updateLastPasser) No kickable." );
        return;
    }

    M_pass_time = M_audio_memory->passTime();
    M_passer_unum = pass.sender_;
    M_receiver_unum = pass.receiver_;
    M_pass_start_pos = ball->pos();
    M_pass_receive_pos = pass.receive_pos_;

    dlog.addText( Logger::WORLD,
                  __FILE__":(updateLastPasser) time=%ld passer=%d receiver=%d start=(%.2f %.2f) end=(%.2f %.2f)",
                  M_pass_time.cycle(),
                  M_passer_unum, M_receiver_unum,
                  M_pass_start_pos.x, M_pass_start_pos.y,
                  M_pass_receive_pos.x, M_pass_receive_pos.y );
}


/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldModel::updateAll( const rcg::DispInfoT & disp )
{
    GameTime new_time = M_time;

    //
    // update current time
    //
    if ( new_time.cycle() == static_cast< long >( disp.show_.time_ ) )
    {
        if ( M_previous_state
             && M_previous_state->gameMode().isServerCycleStoppedMode()
             && M_previous_state->gameMode().getServerPlayMode() == disp.pmode_ )
        {
            new_time.setStopped( M_previous_state->time().stopped() + 1 );
        }
        else
        {
            new_time.assign( static_cast< long >( disp.show_.time_ ), new_time.stopped() + 1 );
        }
    }
    else
    {
        new_time.assign( static_cast< long >( disp.show_.time_ ), 0 );
    }
    M_time = new_time;

    //
    // update playmode
    //
    updateGameMode( disp );

    //
    // update object information
    //
    updateState( disp, new_time );

    //
    // finalize
    //
    updateJustBeforeDecision( new_time );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldModel::updateGameMode( const rcg::DispInfoT & disp )
{
    static const char * s_playmode_strings[] = PLAYMODE_STRINGS;

    if ( disp.pmode_ >= PM_MAX )
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << ": illegal playmode id " << disp.pmode_ << std::endl;
        return;
    }

    std::string mode_str;
    if ( disp.pmode_ == PM_AfterGoal_Left )
    {
        char score[32];
        snprintf( score, 32, "goal_l_%d", disp.team_[0].score() );
        mode_str = score;
    }
    else if ( disp.pmode_ == PM_AfterGoal_Right )
    {
        char score[32];
        snprintf( score, 32, "goal_r_%d", disp.team_[1].score() );
        mode_str = score;
    }
    else
    {
        mode_str = s_playmode_strings[disp.pmode_];
    }

    GameMode new_mode = M_game_mode;
    new_mode.update( mode_str, M_time );

    updateGameMode( new_mode, M_time );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldModel::updateState( const rcg::DispInfoT & disp,
                              const GameTime & current )
{
    dlog.addText( Logger::WORLD,
                  __FILE__": (updateState) [%ld, %ld]", current.cycle(), current.stopped() );

    if ( M_see_time == current )
    {
        return;
    }
    M_see_time = current;

    updateTeamNames( disp );

    M_previous_state = M_current_state;
    M_current_state = CoachWorldState::Ptr( new CoachWorldState( disp,
                                                                 M_time,
                                                                 M_game_mode,
                                                                 M_previous_state ) );

    updatePlayerType( disp );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldModel::updateTeamNames( const rcg::DispInfoT & disp )
{
    if ( M_our_team_name.empty()
         && ! disp.team_[0].name().empty() )
    {
        M_our_team_name = disp.team_[0].name();
    }

    if ( M_their_team_name.empty()
         && ! disp.team_[1].name().empty() )
    {
        M_their_team_name = disp.team_[1].name();
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldModel::updatePlayerType( const rcg::DispInfoT & disp )
{
    //
    // no player type information
    //
    if ( disp.show_.player_[0].type_ < 0 )
    {
        for ( int i = 0; i < 11; ++i )
        {
            M_our_player_type_id[i] = 0;
            M_their_player_type_id[i] = 0;
        }

        return;
    }

    //
    // update player type id array and substitute count
    //
    for ( int i = 0; i < 11; ++i )
    {
        int t = static_cast< int >( disp.show_.player_[i].type_ );
        int o = static_cast< int >( disp.show_.player_[11+i].type_ );

        if ( disp.show_.time_ > 1 )
        {
            if ( M_our_player_type_id[i] != t )
            {
                ++M_our_substitute_count;
            }

            if ( M_their_player_type_id[i] != o )
            {
                ++M_their_substitute_count;
            }

            M_our_player_type_id[i] = t;
            M_their_player_type_id[i] = o;
        }
    }

    //
    // update used count
    //
    const int player_types = PlayerParam::i().playerTypes();
    M_our_player_type_used_count.assign( player_types, 0 );
    M_their_player_type_used_count.assign( player_types, 0 );

    for ( int i = 0; i < 11; ++i )
    {
        {
            const int t =  M_our_player_type_id[i];
            if ( t < player_types )
            {
                M_our_player_type_used_count[t] += 1;
            }
        }
        {
            const int o =  M_their_player_type_id[i];
            if ( o < player_types )
            {
                M_their_player_type_used_count[o] += 1;
            }
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CoachWorldModel::canSubstitute( const int unum,
                                const int type ) const
{
    if ( ourSide() == NEUTRAL )
    {
        // trainer can always substitute any player to any type.
        return true;
    }

    if ( unum < 1 || 11 < unum )
    {
        std::cerr << ourTeamName() << " coach: " << time()
                  << " ***WARNING*** (CoachWorldModel::canSubstitute)"
                  << " illegal uniform number " << unum << std::endl;
        dlog.addText( Logger::WORLD,
                      __FILE__": (canSubstitute) illegal uniform number %d", unum );
        return false;
    }

    if ( type < 0 || PlayerParam::i().playerTypes() <= type )
    {
        std::cerr << ourTeamName() << " coach: " << time()
                  << " ***WARNING*** (CoachWorldModel::canSubstitute)"
                  << " illegal player type id " << type << std::endl;
        dlog.addText( Logger::WORLD,
                      __FILE__": (canSubstitute) illegal player type id %d", type );
        return false;
    }

    if ( this->time().cycle() > 0
         && ourSubstituteCount() >= PlayerParam::i().subsMax() )
    {
        std::cerr << ourTeamName() << " coach: " << time()
                  << " over the substitution max." << std::endl;
        dlog.addText( Logger::WORLD,
                      __FILE__": (canSubstitute) over the substitution max" );
        return false;
    }

    if ( type == Hetero_Default
         && PlayerParam::i().allowMultDefaultType() )
    {
        dlog.addText( Logger::WORLD,
                      __FILE__": (canSubstitute) allow multiple default type" );
        return true;
    }

    const std::vector< int > & used_count = ourPlayerTypeUsedCount();

    try
    {
        if ( used_count.at( type ) >= PlayerParam::i().ptMax() )
        {
            std::cerr << ourTeamName() << " coach: " << time()
                      << " over the ptMax. type=" << type
                      << " used_count=" << used_count[type]
                      << std::endl;
            dlog.addText( Logger::WORLD,
                          __FILE__": (canSubstitute) over the ptMax. type=%d used_count=%d",
                          type, used_count[type] );
            return false;
        }

    }
    catch ( std::exception & e )
    {
        std::cerr << ourTeamName() << " coach: " << time()
                  << " used_count range over "
                  << e.what() << std::endl;
        dlog.addText( Logger::WORLD,
                      __FILE__": (canSubstitute) used_count range over. type=%d", type );
        return false;
    }

    dlog.addText( Logger::WORLD,
                  __FILE__": (canSubstitute) ok. unum=%d type=%d", unum, type );

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
int
CoachWorldModel::playerTypeId( const SideID side,
                               const int unum ) const
{
    if ( side == NEUTRAL )
    {
        std::cerr << ourTeamName() << " coach: " << time()
                  << "***ERROR*** (CoachWorldModel::playerTypeId) "
                  << "invalid side = " << side
                  << std::endl;
        return Hetero_Unknown;
    }

    if ( unum < 1 || 11 < unum )
    {
        std::cerr << ourTeamName() << " coach: " << time()
                  << "***ERROR*** (CoachWorldModel::playerTypeId) "
                  << "invalid unum = " << unum
                  << std::endl;
        return Hetero_Unknown;
    }

    return ( ourSide() == side
             || ( ourSide() == NEUTRAL
                  && side == LEFT )
             ? M_our_player_type_id[unum - 1]
             : M_their_player_type_id[unum - 1] );
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CoachWorldModel::isYellowCarded( const SideID side,
                                 const int unum ) const
{
    if ( side == NEUTRAL
         || unum < 1 || 11 < unum )
    {
        std::cerr << ourTeamName() << " coach: " << time()
                  << "***ERROR*** (CoachWorldModel::isYellowCarded) "
                  << "illegal player id side=" << side
                  << " unum=" << unum << std::endl;
        return false;
    }

    return ( ourSide() == side
             || ( ourSide() == NEUTRAL
                  && side == LEFT )
             ? M_our_card[unum - 1] == YELLOW
             : M_their_card[unum - 1] == YELLOW );
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CoachWorldModel::isRedCarded( const SideID side,
                              const int unum ) const
{
    if ( side == NEUTRAL
         || unum < 1 || 11 < unum )
    {
        std::cerr << ourTeamName() << " coach: " << time()
                  << "***ERROR*** (CoachWorldModel::isRedCarded) "
                  << "illegal player id side=" << side
                  << " unum =" << unum << std::endl;
        return false;
    }

    return ( ourSide() == side
             || ( ourSide() == NEUTRAL
                  && side == LEFT )
             ? M_our_card[unum - 1] == RED
             : M_their_card[unum - 1] == RED );
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CoachWorldModel::canSendCLang( const CLangType type ) const
{
    if ( type == CLANG_MAX_TYPE )
    {
        return false;
    }

    if ( gameMode().type() != GameMode::PlayOn )
    {
        dlog.addText( Logger::WORLD,
                      __FILE__": (decCLangCapacity) non playon. no restriction." );
        return true;
    }

    dlog.addText( Logger::WORLD,
                  __FILE__":(canSendCLang) %s capacity=%d",
                  clang_type_names[type],
                  M_clang_capacity[type] );

    // std::cerr << ourTeamName() << " coach: check clang availability "
    //           << clang_type_names[type] << ". "
    //           << "capacity = " <<  M_clang_capacity[type] << std::endl;

    return M_clang_capacity[type] > 0;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldModel::decCLangCapacity( const CLangType type )
{
    if ( gameMode().type() != GameMode::PlayOn )
    {
        dlog.addText( Logger::WORLD,
                      __FILE__": (decCLangCapacity) non playon. capacity never change." );
        return;
    }

    if ( type == CLANG_UNSUPP
         || type == CLANG_MAX_TYPE )
    {
        return;
    }

    M_clang_capacity[type] = std::max( 0, M_clang_capacity[type] - 1 );

    dlog.addText( Logger::WORLD,
                  __FILE__": (decCLangCapacity) %s capacity=%d",
                  clang_type_names[type], M_clang_capacity[type] );
    std::cerr << ourTeamName() << " coach: " << time()
              << " send clang " << clang_type_names[type]
              << ". new capacity = " <<  M_clang_capacity[type] << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CoachWorldModel::canSendFreeform() const
{
    if ( M_freeform_allowed_count >= 0
         && M_freeform_send_count >= M_freeform_allowed_count )
    {
        return false;
    }

    // if playmode is not playon, coach can send the message anytime.
    if ( gameMode().type() != GameMode::PlayOn )
    {
        return true;
    }

    if ( M_client_version < 7.0 )
    {
        // old version clients can send freeform message only during non playon mode.
        if ( gameMode().type() == GameMode::PlayOn )
        {
            return false;
        }
    }

    // the period that coach can sent the freeform message is very restricted.

    long playon_period = time().cycle() - M_last_playon_start;

    if ( playon_period > ServerParam::i().freeformWaitPeriod() )
    {
        playon_period %= ServerParam::i().freeformWaitPeriod();
        return playon_period < ServerParam::i().freeformSendPeriod();
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldModel::incFreeformSendCount()
{
    ++M_freeform_send_count;

    dlog.addText( Logger::WORLD,
                  __FILE__": (incFreeformSendCount) count=%d / allowd=%d",
                  M_freeform_send_count, M_freeform_allowed_count );
    std::cerr << ourTeamName() << " coach: " << time()
              << " freeform " << M_freeform_send_count
              << " / " << M_freeform_allowed_count
              << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CoachWorldModel::existKickablePlayer() const
{
    const CoachBallObject & cur_ball = M_current_state->ball();

    for ( const CoachPlayerObject * p : allPlayers() )
    {
        int type = playerTypeId( p->side(), p->unum() );
        const PlayerType * param = PlayerTypeSet::i().get( type );
        double kickable_area = ( param
                                 ? param->kickableArea()
                                 : ServerParam::i().defaultKickableArea() );

        if ( p->pos().dist2( cur_ball.pos() ) < std::pow( kickable_area, 2 ) )
        {
            return true;
        }
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

 */
const
CoachPlayerObject *
CoachWorldModel::getPlayerNearestTo( const Vector2D & point ) const
{
    const CoachPlayerObject * ptr = nullptr;
    double max_dist2 = 200000.0;

    for ( const CoachPlayerObject * p : allPlayers() )
    {
        double d2 = p->pos().dist2( point );
        if ( d2 < max_dist2 )
        {
            max_dist2 = d2;
            ptr = p;
        }
    }

    return ptr;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CoachWorldModel::print( std::ostream & os ) const
{
    os << "coach world " << M_time << '\n';


    os << "Ball: "
       << M_current_state->ball().pos() << ' '
       << M_current_state->ball().vel() << '\n';

    for ( const CoachPlayerObject * p : allPlayers() )
    {
        p->print( os ) << '\n';
    }

    os << std::flush;
    return os;
}

}
