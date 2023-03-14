// -*-c++-*-

/*!
  \file see_state.cpp
  \brief see synch state manager Source File
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

#include "see_state.h"

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>

// #define DEBUG_PRINT

namespace rcsc {

bool SeeState::S_synch_see_mode = false;

/*-------------------------------------------------------------------*/
/*!

*/
SeeState::SeeState()
    : M_protocol_version( 1.0 ),
      M_current_time( -1, 0 ),
      M_last_see_time( -1, 0 ),
      M_synch_type( SYNCH_NO ),
      M_last_timing( TIME_NOSYNCH ),
      M_current_see_count( 0 ),
      M_cycles_till_next_see( 100 ),
      M_view_width( ViewWidth::NORMAL ),
      M_view_quality( ViewQuality::HIGH )
{
    for ( int i = 0; i < HISTORY_SIZE; i++ )
    {
        M_see_count_history[i] = 0;
    }
}
/*-------------------------------------------------------------------*/
/*!

*/
SeeState::Timing
SeeState::getNextTiming( const ViewWidth & vw,
                         const ViewQuality & vq ) const
{
    if ( S_synch_see_mode )
    {
        return TIME_SYNC;
    }

    if ( vq.type() == ViewQuality::LOW )
    {
        return TIME_NOSYNCH;
    }

    Timing timing = TIME_NOSYNCH;

    switch ( lastTiming() ) {
    case TIME_0_00:
        switch ( vw.type() ) {
        case ViewWidth::WIDE:
            timing = TIME_0_00;
            break;
        case ViewWidth::NORMAL:
            timing = TIME_50_0;
            break;
        case ViewWidth::NARROW:
            // no synch
            break;
        default:
            break;
        }
        break;
    case TIME_50_0:
        switch ( vw.type() ) {
        case ViewWidth::WIDE:
            timing = TIME_50_0;
            break;
        case ViewWidth::NORMAL:
            timing = TIME_0_00;
            break;
        case ViewWidth::NARROW:
            timing = TIME_22_5;
            break;
        default:
            break;
        }
        break;
    case TIME_22_5:
        switch ( vw.type() ) {
        case ViewWidth::WIDE:
            timing = TIME_22_5;
            break;
        case ViewWidth::NORMAL:
            // no synch
            break;
        case ViewWidth::NARROW:
            timing = TIME_0_00;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    return timing;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
SeeState::updateBySenseBody( const GameTime & sense_time,
                             const ViewWidth & vw,
                             const ViewQuality & vq )
{
    setNewCycle( sense_time );

    if ( M_view_width != vw )
    {
        dlog.addText( Logger::SYSTEM,
                      __FILE__" (updateBySenseBody) view_width does not match. old=%d sense=%d",
                      M_view_width.type(), vw.type() );
        std::cerr << sense_time
                  << " view width does not match . old=" << M_view_width.type()
                  << " sense=" << vw.type()
                  << std::endl;

        M_view_width = vw;
    }

    if ( M_view_quality != vq )
    {
        dlog.addText( Logger::SYSTEM,
                      __FILE__" (updateBySenseBody) view_quality does not match. old=%d sense=%d",
                      M_view_quality.type(), vq.type() );
        std::cerr << sense_time
                  << " view quality does not match. old=" << M_view_quality.type()
                  << " sense=" << vq.type()
                  << std::endl;

        M_view_quality = vq;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
SeeState::updateBySee( const GameTime & see_time,
                       const ViewWidth & vw,
                       const ViewQuality & vq )
{
    // update counter
    if ( see_time == M_last_see_time )
    {
        M_current_see_count += 1;
        if ( isSynch() )
        {
            dlog.addText( Logger::SYSTEM,
                          __FILE__" (updateBySee) estimated synch, but duplicated" );
            std::cerr << see_time
                      << " SeeState.updateSee. estimated synch, but duplicated"
                      << std::endl;
            M_last_timing = TIME_NOSYNCH;
        }
    }
    else
    {
        setNewCycle( see_time );
        M_last_see_time = see_time;
        M_current_see_count = 1;
    }

    // check view quality
    if ( vq == ViewQuality::LOW )
    {
        M_last_timing = TIME_NOSYNCH;
        return;
    }

    if ( ! isSynch() )
    {
        dlog.addText( Logger::SYSTEM,
                      __FILE__" (updateBySee) but no synch" );
        return;
    }

    //
    // see timing is synchronized.
    //

    // update M_cycles_till_next_see and M_synch_type according to the current view mode
    setViewMode( vw, vq );

    // update current see arrival timing.
    Timing new_timing = getNextTiming( vw, vq );
    if ( new_timing == TIME_NOSYNCH )
    {
        std::cerr << see_time
                  << " Invalid view width. no synchronization... "
                  << std::endl;
    }
    dlog.addText( Logger::SYSTEM,
                  __FILE__" (updateBySee) see update, prev timing = %d.  current timing = %d",
                  M_last_timing, new_timing );

    M_last_timing = new_timing;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
SeeState::setNewCycle( const GameTime & new_time )
{
    if ( new_time == M_current_time )
    {
        return;
    }
    M_current_time = new_time;

    if ( --M_cycles_till_next_see < 0 )
    {
        M_cycles_till_next_see = 0;
    }

    // std::rotate( [0], [1], [SIZE] );
    for ( int i = HISTORY_SIZE - 1; i > 0; --i )
    {
        M_see_count_history[i] = M_see_count_history[i-1];
    }
    M_see_count_history[0] = M_current_see_count;
    M_current_see_count = 0;

#ifdef DEBUG_PRINT
    dlog.addText( Logger::SYSTEM,
                  __FILE__" (setNewCycle) set new cycle. cycle till next see = %d",
                  M_cycles_till_next_see );
#endif
}

/*-------------------------------------------------------------------*/
/*!

*/
void
SeeState::setLastSeeTiming( const Timing last_timing )
{
#ifdef DEBUG_PRINT
    dlog.addText( Logger::SYSTEM,
                  __FILE__" (setLastSeeTiming) %d",
                  last_timing );
#endif
    M_last_timing = last_timing;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
SeeState::isSynch() const
{
    if ( S_synch_see_mode )
    {
        return true;
    }

    if ( M_synch_type == SYNCH_SYNC )
    {
        return true;
    }

    return ( M_last_timing == TIME_0_00
             || M_last_timing == TIME_50_0
             || M_last_timing == TIME_22_5 );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
SeeState::isSynchedSeeCountNormalMode() const
{
    return ( M_see_count_history[0] == 2 // previous cycle
             && M_see_count_history[1] == 3 // 2 cycles before
             // && M_see_count_history[2] == 3 // 3 cycles before
             );
    // this condition means last see timing is TIME_0_00
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
SeeState::isSynchedSeeCountSynchMode() const
{
    if ( M_current_see_count == 2 // current cycle
         && M_see_count_history[0] == 3 // previous cycle
         && M_see_count_history[1] == 2 // 2 cycles before
         && M_see_count_history[2] == 3 // 3 cycles before
         )
    {
        // this means last see timing is TIME_50_0
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
SeeState::canSendChangeView( const ViewWidth & next_width,
                             const GameTime & current ) const
{
    //
    // synch mode
    //

    if ( synch_see_mode() )
    {
        if ( current == M_last_see_time )
        {
            return true;
        }

        if ( ServerParam::i().synchSeeOffset() > ServerParam::i().synchOffset()
             && ( M_last_see_time.cycle() + 1 == current.cycle()
                  || ( M_last_see_time.cycle() == current.cycle()
                       && M_last_see_time.stopped() + 1 == current.stopped() ) ) )
        {
            return true;
        }

        return false;
    }

    //
    // no synch mode
    //

    if ( current != M_last_see_time )
    {
        return false;
    }

    if ( next_width.type() == ViewWidth::NARROW )
    {
        if ( lastTiming() == TIME_0_00 )
        {
            return false;
        }
        return true;
    }

    if ( next_width.type() == ViewWidth::NORMAL )
    {
        switch ( lastTiming() ) {
        case TIME_0_00:
        case TIME_50_0:
            return true;
        default:
            return false;
        }

        return false;
    }

    if ( next_width.type() == ViewWidth::WIDE )
    {
        return true;
    }

    std::cerr << __FILE__ << ':' << __LINE__
              << " unexpected reach..." << std::endl;
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
int
SeeState::cyclesTillNextSee() const
{
    return M_cycles_till_next_see;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
SeeState::setViewMode( const ViewWidth & new_width,
                       const ViewQuality & new_quality )
{
    M_view_width = new_width;
    M_view_quality = new_quality;

    if ( synch_see_mode() )
    {
        switch ( new_width.type() ) {
        case ViewWidth::WIDE:
            M_cycles_till_next_see = 3;
            M_synch_type = SYNCH_WIDE;
#ifdef DEBUG_PRINT
            dlog.addText( Logger::SYSTEM,
                          __FILE__" (setViewMode) synch wide: cycle = 3" );
#endif
            break;
        case ViewWidth::NORMAL:
            M_cycles_till_next_see = 2;
            M_synch_type = SYNCH_NORMAL;
#ifdef DEBUG_PRINT
            dlog.addText( Logger::SYSTEM,
                          __FILE__" (setViewMode) synch normal: cycle = 2" );
#endif
            break;
        case ViewWidth::NARROW:
            M_cycles_till_next_see = 1;
            M_synch_type = SYNCH_NARROW;
#ifdef DEBUG_PRINT
            dlog.addText( Logger::SYSTEM,
                          __FILE__" (setViewMode) synch narrow: cycle = 1" );
#endif
            break;
        default:
            break;
        }

//         if ( M_protocol_version >= 18.0 )
//         {
//             M_cycles_till_next_see = 1;
// #ifdef DEBUG_PRINT
//             dlog.addText( Logger::SYSTEM,
//                           __FILE__" (setViewMode) v18+: cycle = 1" );
// #endif
//         }

        return;
    }

    // case 1
    if ( lastTiming() == TIME_0_00 )
    {
        switch ( new_width.type() ) {
        case ViewWidth::WIDE:
            M_cycles_till_next_see = 3;
            M_synch_type = SYNCH_WIDE;
#ifdef DEBUG_PRINT
            dlog.addText( Logger::SYSTEM,
                          __FILE__" (setViewMode) 00:wide: cycle = %d",
                          M_cycles_till_next_see );
#endif
            break;
        case ViewWidth::NORMAL:
            M_cycles_till_next_see = 1;
            M_synch_type = SYNCH_EVERY;
#ifdef DEBUG_PRINT
            dlog.addText( Logger::SYSTEM,
                          __FILE__" (setViewMode) 00:normal: cycle = %d",
                          M_cycles_till_next_see );
#endif
            break;
        case ViewWidth::NARROW:
            std::cerr << M_current_time
                      << " SeeState. TIME_0_00. Narrow is illegal."
                      << std::endl;
            M_synch_type = SYNCH_NO;
            break;
        default:
            break;
        }

        return;
    }

    // case 2
    if ( lastTiming() == TIME_50_0 )
    {
        switch ( new_width.type() ) {
        case ViewWidth::WIDE:
            M_cycles_till_next_see = 3;
            M_synch_type = SYNCH_WIDE;
#ifdef DEBUG_PRINT
            dlog.addText( Logger::SYSTEM,
                          __FILE__" (setViewMode) 50:wide: cycle = %d",
                          M_cycles_till_next_see );
#endif
            break;
        case ViewWidth::NORMAL:
            M_cycles_till_next_see = 2;
            M_synch_type = SYNCH_NORMAL;
#ifdef DEBUG_PRINT
            dlog.addText( Logger::SYSTEM,
                          __FILE__" (setViewMode) 50:normal: cycle = %d",
                          M_cycles_till_next_see );
#endif
            break;
        case ViewWidth::NARROW:
            M_cycles_till_next_see = 1;
            M_synch_type = SYNCH_EVERY;
#ifdef DEBUG_PRINT
            dlog.addText( Logger::SYSTEM,
                          __FILE__" (setViewMode) 50:narrow: cycle = %d",
                          M_cycles_till_next_see );
#endif
            break;
        default:
            break;
        }

        return;
    }

    // case 3
    if ( lastTiming() == TIME_22_5 )
    {
        switch ( new_width.type() ) {
        case ViewWidth::WIDE:
            M_cycles_till_next_see = 3;
            M_synch_type = SYNCH_WIDE;
            break;
        case ViewWidth::NORMAL:
            std::cerr << "SeeState. TIME_22_5. Normal is illegal."
                      << std::endl;
            M_synch_type = SYNCH_NO;
            break;
        case ViewWidth::NARROW:
            M_cycles_till_next_see = 1;
            M_synch_type = SYNCH_EVERY;
            break;
        default:
            break;
        }
        return;
    }

    // case no synchronization...
    M_synch_type = SYNCH_NO;
    switch ( new_width.type() ) {
    case ViewWidth::WIDE:
        M_cycles_till_next_see = 3;
        break;
    case ViewWidth::NORMAL:
        M_cycles_till_next_see = 2;
        break;
    case ViewWidth::NARROW:
        M_cycles_till_next_see = 1;
        break;
    default:
        break;
    }

}

}
