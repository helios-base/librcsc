// -*-c++-*-

/*!
  \file player_object_updater.cpp
  \brief plyaer object update model Source File
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

#include "player_object_updater.h"

#include "localization.h"
#include "self_object.h"
#include "visual_sensor.h"

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/time/timer.h>

// #define DEBUG_PROFILE
// #define DEBUG_PRINT

namespace {

using namespace rcsc;

struct MatchingPair {
    PlayerObject * old_player_;
    std::list< const Localization::PlayerT * > candidates_;

    MatchingPair( PlayerObject * p )
        : old_player_( p )
      { }
};

typedef std::pair< PlayerObject *, const Localization::PlayerT * > ResultPair;


struct MatchingDistanceSorter {
    const Vector2D pos_;

    MatchingDistanceSorter( const Vector2D & pos )
        : pos_( pos )
      { }

    bool operator()( const Localization::PlayerT * lhs,
                     const Localization::PlayerT * rhs ) const
      {
          return lhs->pos_.dist2( pos_ ) < rhs->pos_.dist2( pos_ );
      }
};

struct PlayerUnumSorter {

    bool operator()( const PlayerObject & lhs,
                     const PlayerObject & rhs ) const
      {
          return lhs.unum() < rhs.unum();
      }
};

struct PlayerCountSorter {

    bool operator()( const PlayerObject & lhs,
                     const PlayerObject & rhs ) const
      {
          return lhs.posCount() + lhs.ghostCount() * 10
              < rhs.posCount() + rhs.ghostCount() * 10;
      }
};

struct ResultPairPlayerTEqual {
    const Localization::PlayerT * player_;

    ResultPairPlayerTEqual( const Localization::PlayerT * p )
        : player_( p )
      { }

    bool operator()( const ResultPair & p ) const
      {
          return p.second == player_;
      }
};

/*-------------------------------------------------------------------*/
/*!

 */
void
debug_print_localized_players( const char * header,
                               const std::list< Localization::PlayerT > & teammates,
                               const std::list< Localization::PlayerT > & unknown_teammates,
                               const std::list< Localization::PlayerT > & opponents,
                               const std::list< Localization::PlayerT > & unknown_opponents,
                               const std::list< Localization::PlayerT > & unknown_players )
{
    dlog.addText( Logger::WORLD, "========== %s ==========", header );
    dlog.addText( Logger::WORLD,
                  "seen teammate size = %zd", teammates.size() );
    dlog.addText( Logger::WORLD,
                  "seen unknown teammate size = %zd", unknown_teammates.size() );
    dlog.addText( Logger::WORLD,
                  "seen opponent size = %zd", opponents.size() );
    dlog.addText( Logger::WORLD,
                  "seen unknown opponent size = %zd", unknown_opponents.size() );
    dlog.addText( Logger::WORLD,
                  "seen unknown player size = %zd", unknown_players.size() );
    dlog.addText( Logger::WORLD, "====================" );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
debug_print_player_list_size( const PlayerObject::List & teammates,
                              const PlayerObject::List & opponents,
                              const PlayerObject::List & unknown_players )
{
    dlog.addText( Logger::WORLD,
                  "teammate size = %zd", teammates.size() );
    dlog.addText( Logger::WORLD,
                  "opponent size = %zd", opponents.size() );
    dlog.addText( Logger::WORLD,
                  "unknown  player size = %zd", unknown_players.size() );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
debug_print_all_player_list_size( const char * header,
                                  const PlayerObject::List & teammates,
                                  const PlayerObject::List & opponents,
                                  const PlayerObject::List & unknown_players,
                                  const std::list< Localization::PlayerT > & seen_teammates,
                                  const std::list< Localization::PlayerT > & seen_unknown_teammates,
                                  const std::list< Localization::PlayerT > & seen_opponents,
                                  const std::list< Localization::PlayerT > & seen_unknown_opponents,
                                  const std::list< Localization::PlayerT > & seen_unknown_players )
{
    dlog.addText( Logger::WORLD,"========= %s ========== ", header );

    dlog.addText( Logger::WORLD,
                  "teammate size = %zd", teammates.size() );
    dlog.addText( Logger::WORLD,
                  "opponent size = %zd", opponents.size() );
    dlog.addText( Logger::WORLD,
                  "unknown player size = %zd", unknown_players.size() );

    dlog.addText( Logger::WORLD, "-----" );

    dlog.addText( Logger::WORLD,
                  "seen teammate size = %zd", seen_teammates.size() );
    dlog.addText( Logger::WORLD,
                  "seen unknown teammate size = %zd", seen_unknown_teammates.size() );
    dlog.addText( Logger::WORLD,
                  "seen opponent size = %zd", seen_opponents.size() );
    dlog.addText( Logger::WORLD,
                  "seen unknown opponent size = %zd", seen_unknown_opponents.size() );
    dlog.addText( Logger::WORLD,
                  "seen unknown player size = %zd", seen_unknown_players.size() );

    dlog.addText( Logger::WORLD, "===================" );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
debug_print_player_list( const PlayerObject::List & players,
                         const char * header )


{
    for ( const PlayerObject & p : players )
    {
        dlog.addText( Logger::WORLD,
                      "%s: %d (%.1f %.1f)",
                      header, p.unum(), p.pos().x, p.pos().y );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
debug_print_player_list( const char * header,
                         const PlayerObject::List & teammates,
                         const PlayerObject::List & opponents,
                         const PlayerObject::List & unknown_players )
{
    dlog.addText( Logger::WORLD,
                  "========== %s ==========", header );
    debug_print_player_list_size( teammates, opponents, unknown_players );
    dlog.addText( Logger::WORLD,
                  "----------" );
    debug_print_player_list( teammates, "teammate" );
    debug_print_player_list( opponents, "opponent" );
    debug_print_player_list( unknown_players, "unknown" );
    dlog.addText( Logger::WORLD,
                  "====================" );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
debug_print_matching_pairs( const std::list< MatchingPair > & matching_pairs )
{
    dlog.addText( Logger::WORLD,
                  "debug_print_matching_pairs" );
    for ( const MatchingPair & v = matching_pairs )
    {
        const PlayerObject * p = v.old_player_;
        dlog.addText( Logger::WORLD,
                      "matching_pairs %s %d (%.1f %.1f) candidate %d",
                      side_str( p->side() ), p->unum(), p->pos().x, p->pos().y,
                      it->candidates_.size() );
        for ( const Localization::PlayerT * c : v.candidates_ )
        {
            dlog.addText( Logger::WORLD,
                          "__ candidate %zx %s %d (%.1f %.1f) dist=%f",
                          (size_t)c, side_str( c->side_ ), c->unum_, c->pos_.x, c->pos_.y,
                          p->pos().dist( c->pos_ ) );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
debug_print_result_pairs( const std::vector< ResultPair > & result_pairs )
{
    dlog.addText( Logger::WORLD,
                  "========== matching result pairs ==========" );
    for ( const ResultPair & v : result_pairs )
    {
        dlog.addText( Logger::WORLD,
                      "old: %s %d (%.1f %.1f) <==> seen: %s %d (%.1f %.1f)",
                      side_str( v.first->side() ), v.first->unum(), v.first->pos().x, v.first->pos().y,
                      side_str( v.second->side_ ), v.second->unum_, v.second->pos_.x, v.second->pos_.y );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
create_localized_players_list( const SelfObject & self,
                               const VisualSensor::PlayerCont & seen_players,
                               const SideID seen_side,
                               const Localization * localize,
                               std::list< Localization::PlayerT > * result )
{
    for ( const VisualSensor::PlayerT & p : seen_players )
    {
        result->push_back( Localization::PlayerT() );
        result->back().side_ = seen_side;
        if ( ! localize->localizePlayer( p,
                                         self.face().degree(), self.faceError(),
                                         self.pos(), self.vel(),
                                         &(result->back()) ) )
        {
            result->pop_back();
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
update_unum_matched_players( std::list< Localization::PlayerT > * seen_players,
                             PlayerObject::List * old_players,
                             PlayerObject::List * new_players )
{
    std::list< Localization::PlayerT >::iterator seen = seen_players->begin();
    while ( seen != seen_players->end() )
    {
        if ( seen->unum_ == Unum_Unknown )
        {
            ++seen;
            continue;
        }

        PlayerObject::List::iterator matched = old_players->end();
        for ( PlayerObject::List::iterator p = old_players->begin();
              p != old_players->end();
              ++p )
        {
            if ( p->unum() == seen->unum_ )
            {
                matched = p;
                break;
            }
        }

        if ( matched == old_players->end() )
        {
            ++seen;
            continue;
        }
#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      "unum match: old:% s %d (%.1f %.1f) <--> seen: %s %d (%.1f %.1f)",
                      side_str( matched->side() ), matched->unum(), matched->pos().x, matched->pos().y,
                      side_str( seen->side_ ), seen->unum_, seen->pos_.x, seen->pos_.y );
#endif

        matched->updateBySee( matched->side(), *seen );
        seen = seen_players->erase( seen );
        new_players->splice( new_players->end(), *old_players, matched );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
add_matching_candidates( MatchingPair & result,
                         const std::list< Localization::PlayerT > & seen_players )
{
    const PlayerObject * old_player = result.old_player_;
    const double self_error = 1.2; // magic number
    const double dash_noise = 1.0 + ServerParam::i().playerRand();

    for ( std::list< Localization::PlayerT >::const_iterator seen = seen_players.begin();
          seen != seen_players.end();
          ++seen )
    {
        if ( old_player->unum() != Unum_Unknown
             && seen->unum_ != Unum_Unknown )
        {
// #ifdef DEBUG_PRINT
//             dlog.addText( Logger::WORLD,
//                           "____ add_matching_candidates: different unum. seen = %s %d (%.1f %.1f)",
//                           side_str( seen->side_ ), seen->unum_, seen->pos_.x, seen->pos_.y );
// #endif
            continue; // completely different uniforn number
        }

        int count = old_player->seenPosCount();
        Vector2D old_pos = old_player->seenPos();
        double sensor_error = seen->dist_error_;
        if ( old_player->heardPosCount() < old_player->seenPosCount() )
        {
            count = old_player->heardPosCount();
            old_pos = old_player->heardPos();
            sensor_error = 2.0; // magic number
        }

        double dist2 = seen->pos_.dist2( old_pos );
        if ( dist2 > std::pow( old_player->playerTypePtr()->realSpeedMax() * dash_noise * count
                               + self_error
                               + sensor_error * 3.5, // magic number
                               2 ) )
        {
// #ifdef DEBUG_PRINT
//             dlog.addText( Logger::WORLD,
//                           "____ add_matching_candidates: distance over (%.3f > %.3f). seen = %s %d (%.1f %.1f)",
//                           std::sqrt( dist2 ),
//                           old_player->playerTypePtr()->realSpeedMax() * dash_noise * count
//                           + self_error
//                           + sensor_error * 2.0,
//                           side_str( seen->side_ ), seen->unum_, seen->pos_.x, seen->pos_.y );
// #endif
            continue;
        }

        result.candidates_.push_back( &(*seen) );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
add_matching_pairs( PlayerObject::List & old_players,
                    const std::list< Localization::PlayerT > & seen_players,
                    const std::list< Localization::PlayerT > & seen_unknown_unum_players,
                    const std::list< Localization::PlayerT > & seen_unknown_players,
                    std::list< MatchingPair > * result_list )
{
    for ( PlayerObject::List::iterator p = old_players.begin();
          p != old_players.end();
          ++p )
    {
        result_list->push_back( MatchingPair( &(*p) ) );
        MatchingPair & result = result_list->back();

        add_matching_candidates( result, seen_players );
        add_matching_candidates( result, seen_unknown_unum_players );
        add_matching_candidates( result, seen_unknown_players );

        if ( result.candidates_.empty() )
        {
            result_list->pop_back();
            continue;
        }

        const Vector2D pos = ( p->seenPosCount() <= p->heardPosCount()
                               ? p->seenPos()
                               : p->heardPos() );

        result.candidates_.sort( MatchingDistanceSorter( pos ) );
        if ( result.candidates_.size() > 3 )
        {
            result.candidates_.resize( 3 );
        }

#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      "add_matching_pairs %s %d (%.1f %.1f) candidate %d",
                      side_str( p->side() ), p->unum(), p->pos().x, p->pos().y,
                      result.candidates_.size() );

        for ( const Localization::PlayerT * c : result.candidates_ )
        {
            dlog.addText( Logger::WORLD,
                          "__ candidate %s %d (%.1f %.1f) dist=%.3f",
                          side_str( c->side_ ), c->unum_, c->pos_.x, c->pos_.y,
                          pos.dist( c->pos_ ) );

        }
#endif
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
add_matching_pairs( PlayerObject::List & old_players,
                    const std::list< Localization::PlayerT > & seen_teammates,
                    const std::list< Localization::PlayerT > & seen_unknown_teammates,
                    const std::list< Localization::PlayerT > & seen_opponents,
                    const std::list< Localization::PlayerT > & seen_unknown_opponents,
                    const std::list< Localization::PlayerT > & seen_unknown_players,
                    std::list< MatchingPair > * result_list )
{
    for ( PlayerObject::List::iterator p = old_players.begin();
          p != old_players.end();
          ++p )
    {
        result_list->push_back( MatchingPair( &(*p) ) );

        MatchingPair & result = result_list->back();

        add_matching_candidates( result, seen_teammates );
        add_matching_candidates( result, seen_unknown_teammates );
        add_matching_candidates( result, seen_opponents );
        add_matching_candidates( result, seen_unknown_opponents );
        add_matching_candidates( result, seen_unknown_players );

        if ( result.candidates_.empty() )
        {
            result_list->pop_back();
            continue;
        }

        const Vector2D pos = ( p->seenPosCount() <= p->heardPosCount()
                               ? p->seenPos()
                               : p->heardPos() );

        result.candidates_.sort( MatchingDistanceSorter( pos ) );

#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      "add_matching_pairs: %s %d (%.1f %.1f) candidate %zd",
                      side_str( p->side() ), p->unum(), p->pos().x, p->pos().y,
                      result.candidates_.size() );
        for ( const Localization::PlayerT * c : result.candidates_ )
        {
            dlog.addText( Logger::WORLD,
                          "__ candidate %s %d (%.1f %.1f) dist=%.3f",
                          side_str( c->side_ ), c->unum_, c->pos_.x, c->pos_.y,
                          pos.dist( c->pos_ ) );

        }
#endif
    }

}

/*-------------------------------------------------------------------*/
/*!

 */
bool
exist_duplicated_candidate( const std::list< MatchingPair > & pairs,
                            std::list< MatchingPair >::const_iterator target )
{
    for ( std::list< MatchingPair >::const_iterator it = pairs.begin();
          it != pairs.end();
          ++it )
    {
        if ( it == target ) continue;

        if ( it->candidates_.size() == 1
             && it->candidates_.front() == target->candidates_.front() )
        {
            return true;
        }
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
erase_candidate( std::list< MatchingPair > * matching_pairs,
                 const Localization::PlayerT * candidate )
{
    std::list< MatchingPair >::iterator it = matching_pairs->begin();
    while ( it != matching_pairs->end() )
    {
#if 0
        std::list< const Localization::PlayerT * >::iterator c = it->candidates_.begin();
        while ( c != it->candidates_.end() )
        {
            if ( *c == candidate )
            {
                c = it->candidates_.erase( c );
                break;
            }
            else
            {
                ++c;
            }
        }
#else
        it->candidates_.remove( candidate );
#endif

        if ( it->candidates_.empty() )
        {
            it = matching_pairs->erase( it );
        }
        else
        {
            ++it;
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
find_single_candidate( std::list< MatchingPair > * matching_pairs,
                       std::vector< ResultPair > * result_pairs )
{
#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  "========= start single matching loop ========== " );
#endif

    std::list< MatchingPair >::iterator it = matching_pairs->begin();

    while ( it != matching_pairs->end() )
    {
        if ( it->candidates_.size() == 1 )
        {
            if ( ! exist_duplicated_candidate( *matching_pairs, it ) )
            {
#ifdef DEBUG_PRINT
                dlog.addText( Logger::WORLD,
                              "found matching: old %s %d (%.1f %.1f)",
                              side_str( it->old_player_->side() ),
                              it->old_player_->unum(),
                              it->old_player_->pos().x, it->old_player_->pos().y );
#endif

                const Localization::PlayerT * candidate = it->candidates_.front();

                result_pairs->push_back( ResultPair( it->old_player_, candidate ) );

                matching_pairs->erase( it );
                erase_candidate( matching_pairs, candidate ); // erace the candidate from other list

                it = matching_pairs->begin(); // restart single matching

#ifdef DEBUG_PRINT
                dlog.addText( Logger::WORLD,
                              "========= restart single matching loop ==========" );
                debug_print_matching_pairs( *matching_pairs );
                dlog.addText( Logger::WORLD,
                              "------------------------" );
#endif
                continue;
            }
        }

        ++it;
    }

}

/*-------------------------------------------------------------------*/
/*!

 */
void
evaluate_combination( std::vector< ResultPair > * combination_stack,
                      std::vector< ResultPair > * best_pairs,
                      double * best_value )
{
    double sum_dist2 = 0.0;
    int count = 0;
    for ( std::vector< ResultPair >::const_iterator it = combination_stack->begin();
          it != combination_stack->end();
          ++it )
    {
        const Vector2D & pos = ( it->first->seenPosCount() <= it->first->heardPosCount()
                                 ? it->first->seenPos()
                                 : it->first->heardPos() );
        sum_dist2 += pos.dist2( it->second->pos_ );
        ++count;
    }

    if ( count == 0 )
    {
        return;
    }

    double average_dist2 = sum_dist2 / count;

    if ( *best_value > average_dist2 )
    {
        *best_pairs = *combination_stack;
        *best_value = average_dist2;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
create_combination( std::list< MatchingPair >::iterator first,
                    std::list< MatchingPair >::iterator last,
                    std::vector< ResultPair > * combination_stack,
                    std::vector< ResultPair > * best_pairs,
                    double * best_value )
{
#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  "create_combination stack size=%zd", combination_stack->size() );
#endif
    if ( first == last )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      "create_combination evaluation(1)" );
#endif
        evaluate_combination( combination_stack, best_pairs, best_value );
        return;
    }

    bool found = false;
    for ( std::list< const Localization::PlayerT * >::iterator c = first->candidates_.begin();
          c != first->candidates_.end();
          ++c )
    {
        if ( std::find_if( combination_stack->begin(), combination_stack->end(),
                           ResultPairPlayerTEqual( *c ) )
             == combination_stack->end() )
        {
            found = true;
            combination_stack->push_back( ResultPair( first->old_player_, *c ) );
            create_combination( ++first, last, combination_stack, best_pairs, best_value );
            --first;
            combination_stack->pop_back();
        }
    }

    if ( ! found )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      "create_combination evaluation(2)" );
#endif
        evaluate_combination( combination_stack, best_pairs, best_value );
    }
}


/*-------------------------------------------------------------------*/
/*!

 */
void
find_best_combination( std::list< MatchingPair > * matching_pairs,
                       std::vector< ResultPair > * result_pairs )
{
    if ( matching_pairs->empty() )
    {
        return;
    }

#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  "========= start find best combination ==========" );
#endif

    std::vector< ResultPair > combination_stack;
    std::vector< ResultPair > best_result;
    combination_stack.reserve( matching_pairs->size() );
    best_result.reserve( matching_pairs->size() );

    double best_value = 100000000.0;
    create_combination( matching_pairs->begin(), matching_pairs->end(),
                        &combination_stack,
                        &best_result,
                        &best_value );
#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  "best pair: value=%f", best_value );
    for ( std::vector< ResultPair >::const_iterator it = best_result.begin();
          it != best_result.end();
          ++it )
    {
        dlog.addText( Logger::WORLD,
                      "__ old: %s %d (%.1f %.1f) <==> seen: %s %d (%.1f %.1f)",
                      side_str( it->first->side() ), it->first->unum(), it->first->pos().x, it->first->pos().y,
                      side_str( it->second->side_ ), it->second->unum_, it->second->pos_.x, it->second->pos_.y );
    }
#endif

    // append the best combination
    result_pairs->insert( result_pairs->end(), best_result.begin(), best_result.end() );
}


/*-------------------------------------------------------------------*/
/*!

 */
void
find_nearest_candidate( std::list< MatchingPair > * matching_pairs,
                        std::vector< ResultPair > * result_pairs )
{
#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  "========= start nearest matching loop ========== " );
#endif

    while ( ! matching_pairs->empty() )
    {
        double min_dist2 = 10000000.0;
        std::list< MatchingPair >::iterator best_it = matching_pairs->end();

        for ( std::list< MatchingPair >::iterator it = matching_pairs->begin();
              it != matching_pairs->end();
              ++it )
        {
            if ( it->candidates_.empty() ) continue;

            Vector2D pos = ( it->old_player_->seenPosCount() <= it->old_player_->heardPosCount()
                             ? it->old_player_->seenPos()
                             : it->old_player_->heardPos() );
            double d2 = pos.dist2( it->candidates_.front()->pos_ );
            if ( d2 < min_dist2 )
            {
                min_dist2 = d2;
                best_it = it;
            }
        }

        if ( best_it == matching_pairs->end() )
        {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      "nearest: not found" );
#endif
            break;
        }

        const Localization::PlayerT * best_seen = best_it->candidates_.front();

        result_pairs->push_back( ResultPair( best_it->old_player_, best_seen ) );

#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      "nearest: old %s %d (%.1f %.1f) dist=%.3f",
                      side_str( best_it->old_player_->side() ),
                      best_it->old_player_->unum(),
                      best_it->old_player_->pos().x, best_it->old_player_->pos().y,
                      std::sqrt( min_dist2 ) );
#endif

        matching_pairs->erase( best_it );
        erase_candidate( matching_pairs, best_seen );
    }

#ifdef DEBUG_PRINT
    if ( ! matching_pairs->empty() )
    {
        dlog.addText( Logger::WORLD,
                      "------------------------" );
        dlog.addText( Logger::WORLD,
                      "xxxxx remained pairs %zd", matching_pairs->size() );
        debug_print_matching_pairs( *matching_pairs );
        dlog.addText( Logger::WORLD,
                      "------------------------" );
    }
#endif
}

/*-------------------------------------------------------------------*/
/*!

 */
void
update_result_pairs( std::vector< ResultPair > * result_pairs )
{
    for ( std::vector< ResultPair >::iterator it = result_pairs->begin(), end = result_pairs->end();
          it != end;
          ++it )
    {
        SideID side = ( it->first->side() != NEUTRAL
                        ? it->first->side()
                        : it->second->side_ );

        it->first->updateBySee( side, *(it->second) );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
erase_seen_player( const Localization::PlayerT * p,
                   std::list< Localization::PlayerT > * players )
{
    std::list< Localization::PlayerT >::iterator it = players->begin();
    while ( it != players->end() )
    {
        if ( &(*it) == p )
        {
#ifdef DEBUG_PRINT
            dlog.addText( Logger::WORLD,
                          "erase: %s %d (%.1f %.1f)",
                      side_str( p->side_ ), p->unum_, p->pos_.x, p->pos_.y );
#endif
            it = players->erase( it );
            return;
        }

        ++it;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
erase_matched_seen_players( const SideID our_side,
                            const std::vector< ResultPair > & result_pairs,
                            std::list< Localization::PlayerT > * seen_teammates,
                            std::list< Localization::PlayerT > * seen_unknown_teammates,
                            std::list< Localization::PlayerT > * seen_opponents,
                            std::list< Localization::PlayerT > * seen_unknown_opponents,
                            std::list< Localization::PlayerT > * seen_unknown_players )
{
    for ( std::vector< ResultPair >::const_iterator it = result_pairs.begin();
          it != result_pairs.end();
          ++it )
    {
        if ( it->second->side_ == NEUTRAL )
        {
            erase_seen_player( it->second, seen_unknown_players );
        }
        else if ( it->second->side_ == our_side )
        {
            if ( it->second->unum_ != Unum_Unknown )
            {
                erase_seen_player( it->second, seen_teammates );
            }
            else
            {
                erase_seen_player( it->second, seen_unknown_teammates );
            }
        }
        else
        {
            if ( it->second->unum_ != Unum_Unknown )
            {
                erase_seen_player( it->second, seen_opponents );
            }
            else
            {
                erase_seen_player( it->second, seen_unknown_opponents );
            }
        }
    }
}

} // end anonymous namespace

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
bool
PlayerObjectUpdater::localizePlayers( const SelfObject & self,
                                      const VisualSensor & see,
                                      const Localization * localize,
                                      PlayerObject::List & teammates,
                                      PlayerObject::List & opponents,
                                      PlayerObject::List & unknown_players )
{
#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  "========== (localizePlayers) ==========" );
#endif

    if ( ! self.faceValid()
         || ! self.posValid() )
    {
        return false;
    }

#ifdef DEBUG_PROFILE
    Timer timer;
#endif

#ifdef DEBUG_PRINT
    debug_print_player_list( "initial list", teammates, opponents, unknown_players );
#endif
    const SideID our_side = self.side();
    const SideID their_side = ( our_side == LEFT ? RIGHT : LEFT );

    PlayerObject::List new_teammates;
    PlayerObject::List new_opponents;

    std::list< Localization::PlayerT > seen_teammates;
    std::list< Localization::PlayerT > seen_unknown_teammates;
    std::list< Localization::PlayerT > seen_opponents;
    std::list< Localization::PlayerT > seen_unknown_opponents;
    std::list< Localization::PlayerT > seen_unknown_players;

    //
    // create localized players
    //
    create_localized_players_list( self, see.teammates(),        our_side,   localize, &seen_teammates );
    create_localized_players_list( self, see.unknownTeammates(), our_side,   localize, &seen_unknown_teammates );
    create_localized_players_list( self, see.opponents(),        their_side, localize, &seen_opponents );
    create_localized_players_list( self, see.unknownOpponents(), their_side, localize, &seen_unknown_opponents );
    create_localized_players_list( self, see.unknownPlayers(),   NEUTRAL,    localize, &seen_unknown_players );

#ifdef DEBUG_PRINT
    debug_print_localized_players( "after the localization",
                                   seen_teammates, seen_unknown_teammates,
                                   seen_opponents, seen_unknown_opponents, seen_unknown_players );
#endif

    //
    // uniform number matching
    //
#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD, "========== unifor number matching ==========" );
#endif

    update_unum_matched_players( &seen_teammates, &teammates, &new_teammates );
    update_unum_matched_players( &seen_opponents, &opponents, &new_opponents );

#ifdef DEBUG_PRINT
    debug_print_all_player_list_size( "after the uniform number matching",
                                      teammates, opponents, unknown_players,
                                      seen_teammates, seen_unknown_teammates,
                                      seen_opponents, seen_unknown_opponents, seen_unknown_players );
#endif

    //
    // matching
    //

#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  "========= create matching pairs ========== " );
#endif
    std::list< MatchingPair > matching_pairs;

    add_matching_pairs( teammates,
                        seen_teammates, seen_unknown_teammates, seen_unknown_players,
                        &matching_pairs );
    add_matching_pairs( opponents,
                        seen_opponents, seen_unknown_opponents, seen_unknown_players,
                        &matching_pairs );
    add_matching_pairs( unknown_players,
                        seen_teammates, seen_unknown_teammates,
                        seen_opponents, seen_unknown_opponents, seen_unknown_players,
                        &matching_pairs );

#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  "========= start matching loop ========== " );
    debug_print_matching_pairs( matching_pairs );
#endif

    std::vector< ResultPair > result_pairs;

    //
    // find the single candidate
    //
    find_single_candidate( &matching_pairs, &result_pairs );

    //
    // find the nearest candidate
    //
    //find_nearest_candidate( &matching_pairs, &result_pairs );
    find_best_combination( &matching_pairs, &result_pairs );

#ifdef DEBUG_PRINT
    debug_print_result_pairs( result_pairs );
#endif

    //
    // update by seen information
    //
    update_result_pairs( &result_pairs );

    //
    // erase matched seen players from list
    //
    erase_matched_seen_players( our_side, result_pairs,
                                &seen_teammates, &seen_unknown_teammates,
                                &seen_opponents, &seen_unknown_opponents, &seen_unknown_players );

    //
    // add new players if seen players still exist.
    //
#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  "========== add new players ==========" );
#endif

    addNewPlayers( our_side,   seen_teammates,         new_teammates );
    addNewPlayers( our_side,   seen_unknown_teammates, new_teammates );
    addNewPlayers( their_side, seen_opponents,         new_opponents );
    addNewPlayers( their_side, seen_unknown_opponents, new_opponents );
    addNewPlayers( NEUTRAL,    seen_unknown_players,   unknown_players ); // unknown players are directry added to the existing list

    // splice to exsiting list
    teammates.splice( teammates.end(), new_teammates );
    opponents.splice( opponents.end(), new_opponents );

#ifdef DEBUG_PROFILE
    dlog.addText( Logger::WORLD,
                  __FILE__":(localizePlayers) elpased %lf [ms]", timer.elapsedReal() );
#endif

#ifdef DEBUG_PRINT
    teammates.sort( PlayerUnumSorter() );
    opponents.sort( PlayerUnumSorter() );
    unknown_players.sort( PlayerCountSorter() );
    debug_print_player_list( "result list", teammates, opponents, unknown_players );
#endif

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerObjectUpdater::addNewPlayers( const SideID side,
                                    const std::list< Localization::PlayerT > & seen_players,
                                    PlayerObject::List & result_list )
{
    for ( std::list< Localization::PlayerT >::const_iterator it = seen_players.begin();
          it != seen_players.end();
          ++it )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      "add: %s %d (%.1f %.1f)",
                      side_str( side ), it->unum_, it->pos_.x, it->pos_.y );
#endif
        result_list.push_back( PlayerObject( side, *it ) ); // increment player count
    }
}

}
