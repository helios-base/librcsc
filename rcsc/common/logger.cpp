// -*-c++-*-

/*!
  \file logger.cpp
  \brief Logger class Source File
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

#include "logger.h"

#include <rcsc/game_time.h>

#include <string>
#include <iostream>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <mutex>
#include <memory>
#include <atomic>

namespace rcsc {

namespace  {

//! buffer size for the log message.
constexpr size_t G_BUFFER_SIZE = 2048;

//! main buffer - now thread-safe with mutex protection
struct ThreadSafeBuffer {
    std::string buffer;
    std::mutex mutex;
    
    ThreadSafeBuffer() {
        buffer.reserve(8192 * 4);
    }
    
    void append(const std::string& str) {
        std::lock_guard<std::mutex> lock(mutex);
        buffer += str;
    }
    
    void clear() {
        std::lock_guard<std::mutex> lock(mutex);
        buffer.clear();
    }
    
    std::string extract() {
        std::lock_guard<std::mutex> lock(mutex);
        std::string result = std::move(buffer);
        buffer.clear();
        return result;
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex);
        return buffer.size();
    }
};

//! thread-safe global buffer
static ThreadSafeBuffer g_thread_safe_buffer;

//! temporary buffer - thread-local for better performance
thread_local char g_buffer[G_BUFFER_SIZE];

}

//! global variable - now thread-safe
static std::atomic<Logger*> g_global_logger{nullptr};

//! global logger instance
Logger dlog;

/*-------------------------------------------------------------------*/
/*!

 */
Logger::Logger()
    : M_time( nullptr ),
      M_fout( nullptr ),
      M_flags( 0 ),
      M_start_time( -1 ),
      M_end_time( 99999999 )
{
    // Initialize thread-local buffer
    std::memset(g_buffer, 0, G_BUFFER_SIZE);
    
    // Set as global logger atomically
    Logger* expected = nullptr;
    g_global_logger.compare_exchange_strong(expected, this);
}

/*-------------------------------------------------------------------*/
/*!

 */
Logger::~Logger()
{
    close();
    
    // Clear global logger reference if it's this instance
    Logger* expected = this;
    g_global_logger.compare_exchange_strong(expected, nullptr);
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Logger::setLogFlag( const GameTime * time,
                    const std::int32_t level,
                    const bool on )
{
    M_time = time;

    if ( on )
    {
        M_flags |= level;
    }
    else
    {
        M_flags &= ~level;
    }
}

/*-------------------------------------------------------------------*/
void
Logger::setTimeRange( const int start_time,
                      const int end_time )
{
    M_start_time = start_time;
    M_end_time = end_time;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Logger::close()
{
    if ( M_fout )
    {
        flush();
        if ( M_fout != stdout
             && M_fout != stderr )
        {
            fclose( M_fout );
        }
        M_fout = nullptr;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Logger::open( const std::string & filepath )
{
    close();

    M_fout = std::fopen( filepath.c_str(), "w" );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Logger::openStandardOutput()
{
    close();

    M_fout = stdout;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Logger::openStandardError()
{
    close();

    M_fout = stderr;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Logger::flush()
{
    if ( M_fout && g_thread_safe_buffer.size() > 0 )
    {
        std::string data = g_thread_safe_buffer.extract();
        if (!data.empty()) {
            fputs( data.c_str(), M_fout );
            fflush( M_fout );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Logger::clear()
{
    g_thread_safe_buffer.clear();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Logger::addText( const std::int32_t level,
                 const char * msg,
                 ... )
{
    if ( M_fout
         && M_time
         && ( level & M_flags )
         && M_start_time <= M_time->cycle()
         && M_time->cycle() <= M_end_time )
    {
        va_list argp;
        va_start( argp, msg );
        
        // Use vsnprintf with proper bounds checking
        int result = vsnprintf( g_buffer, G_BUFFER_SIZE, msg, argp );
        va_end( argp );
        
        if (result < 0 || static_cast<size_t>(result) >= G_BUFFER_SIZE) {
            // Handle truncation or error
            g_buffer[G_BUFFER_SIZE - 1] = '\0';
        }

        char header[64];
        snprintf( header, sizeof(header), "%ld,%ld %d M ",
                  M_time->cycle(),
                  M_time->stopped(),
                  level );

        std::string log_entry = header + std::string(g_buffer) + '\n';
        g_thread_safe_buffer.append(log_entry);
        
        // Flush if buffer gets too large
        if ( g_thread_safe_buffer.size() > 8192 * 3 )
        {
            flush();
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Logger::addPoint( const std::int32_t level,
                  const double x,
                  const double y,
                  const char * color )
{
    if ( M_fout
         && M_time
         && ( level & M_flags )
         && M_start_time <= M_time->cycle()
         && M_time->cycle() <= M_end_time )
    {
        char msg[128];
        snprintf( msg, 128, "%ld,%ld %d p %.4f %.4f ",
                  M_time->cycle(),
                  M_time->stopped(),
                  level,
                  x, y );
        g_thread_safe_buffer.append(msg);
        if ( color )
        {
            g_thread_safe_buffer.append(color);
        }
        g_thread_safe_buffer.append("\n");
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Logger::addPoint( const std::int32_t level,
                  const double x,
                  const double y,
                  const int r, const int g, const int b )
{
    if ( M_fout
         && M_time
         && ( level & M_flags )
         && M_start_time <= M_time->cycle()
         && M_time->cycle() <= M_end_time )
    {
        char msg[128];
        snprintf( msg, 128, "%ld,%ld %d p %.4f %.4f #%02x%02x%02x",
                  M_time->cycle(),
                  M_time->stopped(),
                  level,
                  x, y,
                  r, g, b );
        g_thread_safe_buffer.append(msg);
        g_thread_safe_buffer.append("\n");
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Logger::addLine( const std::int32_t level,
                 const double x1,
                 const double y1,
                 const double x2,
                 const double y2,
                 const char * color )
{
    if ( M_fout
         && M_time
         && ( level & M_flags )
         && M_start_time <= M_time->cycle()
         && M_time->cycle() <= M_end_time )
    {
        char msg[128];
        snprintf( msg, 128, "%ld,%ld %d l %.4f %.4f %.4f %.4f ",
                  M_time->cycle(),
                  M_time->stopped(),
                  level,
                  x1, y1, x2, y2 );
        g_thread_safe_buffer.append(msg);
        if ( color )
        {
            g_thread_safe_buffer.append(color);
        }
        g_thread_safe_buffer.append("\n");
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Logger::addLine( const std::int32_t level,
                 const double x1,
                 const double y1,
                 const double x2,
                 const double y2,
                 const int r, const int g, const int b )
{
    if ( M_fout
         && M_time
         && ( level & M_flags )
         && M_start_time <= M_time->cycle()
         && M_time->cycle() <= M_end_time )
    {
        char msg[128];
        snprintf( msg, 128, "%ld,%ld %d l %.4f %.4f %.4f %.4f #%02x%02x%02x",
                  M_time->cycle(),
                  M_time->stopped(),
                  level,
                  x1, y1, x2, y2,
                  r, g, b );
        g_thread_safe_buffer.append(msg);
        g_thread_safe_buffer.append("\n");
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Logger::addArc( const std::int32_t level,
                const double x,
                const double y,
                const double radius,
                const AngleDeg & start_angle,
                const double span_angle,
                const char * color )
{
    if ( M_fout
         && M_time
         && ( level & M_flags )
         && M_start_time <= M_time->cycle()
         && M_time->cycle() <= M_end_time )
    {
        char msg[128];
        snprintf( msg, 128, "%ld,%ld %d a %.4f %.4f %.4f %.4f %.4f ",
                  M_time->cycle(),
                  M_time->stopped(),
                  level,
                  x, y, radius, start_angle.degree(), span_angle );
        g_thread_safe_buffer.append(msg);

        if ( color )
        {
            g_thread_safe_buffer.append(color);
        }

        g_thread_safe_buffer.append("\n");
    }
}


/*-------------------------------------------------------------------*/
/*!

 */
void
Logger::addArc( const std::int32_t level,
                const double x,
                const double y,
                const double radius,
                const AngleDeg & start_angle,
                const double span_angle,
                const int r, const int g, const int b )
{
    if ( M_fout
         && M_time
         && ( level & M_flags )
         && M_start_time <= M_time->cycle()
         && M_time->cycle() <= M_end_time )
    {
        char msg[128];
        snprintf( msg, 128, "%ld,%ld %d a %.4f %.4f %.4f %.4f %.4f #%02x%02x%02x",
                  M_time->cycle(),
                  M_time->stopped(),
                  level,
                  x, y, radius, start_angle.degree(), span_angle,
                  r, g, b );
        g_thread_safe_buffer.append(msg);
        g_thread_safe_buffer.append("\n");
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Logger::addCircle( const std::int32_t level,
                   const double x,
                   const double y,
                   const double radius,
                   const char * color,
                   const bool fill )
{
    if ( M_fout
         && M_time
         && ( level & M_flags )
         && M_start_time <= M_time->cycle()
         && M_time->cycle() <= M_end_time )
    {
        char msg[128];
        snprintf( msg, 128, "%ld,%ld %d %c %.4f %.4f %.4f ",
                  M_time->cycle(),
                  M_time->stopped(),
                  level,
                  ( fill ? 'C' : 'c' ),
                  x, y, radius );
        g_thread_safe_buffer.append(msg);
        if ( color )
        {
            g_thread_safe_buffer.append(color);
        }
        g_thread_safe_buffer.append("\n");
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Logger::addCircle( const std::int32_t level,
                   const double x,
                   const double y,
                   const double radius,
                   const int r, const int g, const int b,
                   const bool fill )
{
    if ( M_fout
         && M_time
         && ( level & M_flags )
         && M_start_time <= M_time->cycle()
         && M_time->cycle() <= M_end_time )
    {
        char msg[128];
        snprintf( msg, 128, "%ld,%ld %d %c %.4f %.4f %.4f #%02x%02x%02x",
                  M_time->cycle(),
                  M_time->stopped(),
                  level,
                  ( fill ? 'C' : 'c' ),
                  x, y, radius,
                  r, g, b );
        g_thread_safe_buffer.append(msg);
        g_thread_safe_buffer.append("\n");
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Logger::addTriangle( const std::int32_t level,
                     const double x1,
                     const double y1,
                     const double x2,
                     const double y2,
                     const double x3,
                     const double y3,
                     const char * color,
                     const bool fill )
{
    if ( M_fout
         && M_time
         && ( level & M_flags )
         && M_start_time <= M_time->cycle()
         && M_time->cycle() <= M_end_time )
    {
        char msg[128];
        snprintf( msg, 128, "%ld,%ld %d %c %.4f %.4f %.4f %.4f %.4f %.4f ",
                  M_time->cycle(),
                  M_time->stopped(),
                  level,
                  ( fill ? 'T' : 't' ),
                  x1, y1, x2, y2, x3, y3 );
        g_thread_safe_buffer.append(msg);
        if ( color )
        {
            g_thread_safe_buffer.append(color);
        }
        g_thread_safe_buffer.append("\n");
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Logger::addTriangle( const std::int32_t level,
                     const double x1,
                     const double y1,
                     const double x2,
                     const double y2,
                     const double x3,
                     const double y3,
                     const int r, const int g, const int b,
                     const bool fill )
{
    if ( M_fout
         && M_time
         && ( level & M_flags )
         && M_start_time <= M_time->cycle()
         && M_time->cycle() <= M_end_time )
    {
        char msg[128];
        snprintf( msg, 128, "%ld,%ld %d %c %.4f %.4f %.4f %.4f %.4f %.4f #%02x%02x%02x",
                  M_time->cycle(),
                  M_time->stopped(),
                  level,
                  ( fill ? 'T' : 't' ),
                  x1, y1, x2, y2, x3, y3,
                  r, g, b );
        g_thread_safe_buffer.append(msg);
        g_thread_safe_buffer.append("\n");
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Logger::addRect( const std::int32_t level,
                 const double left,
                 const double top,
                 const double length,
                 const double width,
                 const char * color,
                 const bool fill )
{
    if ( M_fout
         && M_time
         && ( level & M_flags )
         && M_start_time <= M_time->cycle()
         && M_time->cycle() <= M_end_time )
    {
        char msg[128];
        snprintf( msg, 128, "%ld,%ld %d %c %.4f %.4f %.4f %.4f ",
                  M_time->cycle(),
                  M_time->stopped(),
                  level,
                  ( fill ? 'R' : 'r' ),
                  left, top, length, width );
        g_thread_safe_buffer.append(msg);
        if ( color )
        {
            g_thread_safe_buffer.append(color);
        }
        g_thread_safe_buffer.append("\n");
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Logger::addRect( const std::int32_t level,
                 const double left,
                 const double top,
                 const double length,
                 const double width,
                 const int r, const int g, const int b,
                 const bool fill )
{
    if ( M_fout
         && M_time
         && ( level & M_flags )
         && M_start_time <= M_time->cycle()
         && M_time->cycle() <= M_end_time )
    {
        char msg[128];
        snprintf( msg, 128, "%ld,%ld %d %c %.4f %.4f %.4f %.4f #%02x%02x%02x",
                  M_time->cycle(),
                  M_time->stopped(),
                  level,
                  ( fill ? 'R' : 'r' ),
                  left, top, length, width,
                  r, g, b );
        g_thread_safe_buffer.append(msg);
        g_thread_safe_buffer.append("\n");
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Logger::addSector( const std::int32_t level,
                   const double x,
                   const double y,
                   const double min_radius,
                   const double max_radius,
                   const AngleDeg & start_angle,
                   const double span_angle,
                   const char * color,
                   const bool fill )
{
    if ( M_fout
         && M_time
         && ( level & M_flags )
         && M_start_time <= M_time->cycle()
         && M_time->cycle() <= M_end_time )
    {
        char msg[128];
        snprintf( msg, 128, "%ld,%ld %d %c %.4f %.4f %.4f %.4f %.4f %.4f ",
                  M_time->cycle(),
                  M_time->stopped(),
                  level,
                  ( fill ? 'S' : 's' ),
                  x, y, min_radius, max_radius,
                  start_angle.degree(), span_angle );
        g_thread_safe_buffer.append(msg);
        if ( color )
        {
            g_thread_safe_buffer.append(color);
        }
        g_thread_safe_buffer.append("\n");
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Logger::addSector( const std::int32_t level,
                   const double x,
                   const double y,
                   const double min_radius,
                   const double max_radius,
                   const AngleDeg & start_angle,
                   const double span_angle,
                   const int r, const int g, const int b,
                   const bool fill )
{
    if ( M_fout
         && M_time
         && ( level & M_flags )
         && M_start_time <= M_time->cycle()
         && M_time->cycle() <= M_end_time )
    {
        char msg[128];
        snprintf( msg, 128, "%ld,%ld %d %c %.4f %.4f %.4f %.4f %.4f %.4f #%02x%02x%02x",
                  M_time->cycle(),
                  M_time->stopped(),
                  level,
                  ( fill ? 'S' : 's' ),
                  x, y, min_radius, max_radius,
                  start_angle.degree(), span_angle,
                  r, g, b );
        g_thread_safe_buffer.append(msg);
        g_thread_safe_buffer.append("\n");
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Logger::addSector( const std::int32_t level,
                   const Sector2D & sector,
                   const char * color,
                   const bool fill )
{
    if ( M_fout
         && M_time
         && ( level & M_flags )
         && M_start_time <= M_time->cycle()
         && M_time->cycle() <= M_end_time )
    {
        char msg[128];
        double span_angle = ( sector.angleLeftStart().isLeftOf( sector.angleRightEnd() )
                              ? ( sector.angleLeftStart() - sector.angleRightEnd() ).abs()
                              : 360.0 - ( sector.angleLeftStart() - sector.angleRightEnd() ).abs() );
        snprintf( msg, 128, "%ld,%ld %d %c %.4f %.4f %.4f %.4f %.4f %.4f ",
                  M_time->cycle(),
                  M_time->stopped(),
                  level,
                  ( fill ? 'S' : 's' ),
                  sector.center().x, sector.center().y,
                  sector.radiusMin(), sector.radiusMax(),
                  sector.angleLeftStart().degree(), span_angle );
        g_thread_safe_buffer.append(msg);
        if ( color )
        {
            g_thread_safe_buffer.append(color);
        }
        g_thread_safe_buffer.append("\n");
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Logger::addSector( const std::int32_t level,
                   const Sector2D & sector,
                   const int r, const int g, const int b,
                   const bool fill )
{
    if ( M_fout
         && M_time
         && ( level & M_flags )
         && M_start_time <= M_time->cycle()
         && M_time->cycle() <= M_end_time )
    {
        char msg[128];
        double span_angle = ( sector.angleLeftStart().isLeftOf( sector.angleRightEnd() )
                              ? ( sector.angleLeftStart() - sector.angleRightEnd() ).abs()
                              : 360.0 - ( sector.angleLeftStart() - sector.angleRightEnd() ).abs() );
        snprintf( msg, 128, "%ld,%ld %d %c %.4f %.4f %.4f %.4f %.4f %.4f #%02x%02x%02x",
                  M_time->cycle(),
                  M_time->stopped(),
                  level,
                  ( fill ? 'S' : 's' ),
                  sector.center().x, sector.center().y,
                  sector.radiusMin(), sector.radiusMax(),
                  sector.angleLeftStart().degree(), span_angle,
                  r, g, b );
        g_thread_safe_buffer.append(msg);
        g_thread_safe_buffer.append("\n");
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Logger::addMessage( const std::int32_t level,
                    const double x,
                    const double y,
                    const char * msg,
                    const char * color )
{
    if ( M_fout
         && M_time
         && ( level & M_flags )
         && M_start_time <= M_time->cycle()
         && M_time->cycle() <= M_end_time )
    {
        char header[128];
        snprintf( header, 128, "%ld,%ld %d m %.4f %.4f ",
                  M_time->cycle(),
                  M_time->stopped(),
                  level,
                  x, y );
        g_thread_safe_buffer.append(header);

        if ( color )
        {
            g_thread_safe_buffer.append("(c ");
            g_thread_safe_buffer.append(color);
            g_thread_safe_buffer.append(") ");
        }

        g_thread_safe_buffer.append(msg);
        g_thread_safe_buffer.append("\n");
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Logger::addMessage( const std::int32_t level,
                    const double x,
                    const double y,
                    const char * msg,
                    const int r, const int g, const int b )
{
    if ( M_fout
         && M_time
         && ( level & M_flags )
         && M_start_time <= M_time->cycle()
         && M_time->cycle() <= M_end_time )
    {
        char header[128];
        snprintf( header, 128, "%ld,%ld %d m %.4f %.4f ",
                  M_time->cycle(),
                  M_time->stopped(),
                  level,
                  x, y );
        g_thread_safe_buffer.append(header);

        char col[8];
        snprintf( col, 8, "#%02x%02x%02x", r, g, b );
        g_thread_safe_buffer.append("(c ");
        g_thread_safe_buffer.append(col);
        g_thread_safe_buffer.append(") ");

        g_thread_safe_buffer.append(msg);
        g_thread_safe_buffer.append("\n");
    }
}

}
