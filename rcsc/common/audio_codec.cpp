// -*-c++-*-

/*!
  \file audio_codec.cpp
  \brief audio message encoder/decoder Source File
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

#include "audio_codec.h"

#include "server_param.h"

#include <rcsc/math_util.h>
#include <rcsc/types.h>

#include <iostream>
#include <algorithm>
#include <limits>

namespace {

const double X_NORM_FACTOR = 57.5; //!< x normalize factor (field length) to limit inputed x
const double Y_NORM_FACTOR = 39.0; //!< y normalze factor (field width) to limit inputed y
const double SPEED_NORM_FACTOR = 3.0; //!< speed normalize factor to limit inputed speed range

const double COORD_STEP_L2 = 0.1; //!< used by encodeCoordToStr2/decodeStr2ToCoord
const double SPEED_STEP_L1 = 0.1; //!< used by encodeSpeedToChar/decodeCharToSpeed


/*!
  \brief bit mask enumeration
*/
enum {
    MASK_1 = 0x00000001,
    MASK_2 = 0x00000003,
    MASK_3 = 0x00000007,
    MASK_4 = 0x0000000F,
    MASK_5 = 0x0000001F,
    MASK_6 = 0x0000003F,
    MASK_7 = 0x0000007F,
    MASK_8 = 0x000000FF,
    MASK_9 = 0x000001FF,
    MASK_10 = 0x000003FF,
    MASK_11 = 0x000007FF,
    MASK_12 = 0x00000FFF,
    MASK_13 = 0x00001FFF,
    MASK_14 = 0x00003FFF,
    MASK_15 = 0x00007FFF,
    MASK_16 = 0x0000FFFF,
    MASK_17 = 0x0001FFFF,
    MASK_18 = 0x0003FFFF,
    MASK_19 = 0x0007FFFF,
    MASK_20 = 0x000FFFFF,
    MASK_21 = 0x001FFFFF,
    MASK_22 = 0x003FFFFF,
    MASK_23 = 0x007FFFFF,
    MASK_24 = 0x00FFFFFF,
    MASK_25 = 0x01FFFFFF,
    MASK_26 = 0x03FFFFFF,
    MASK_27 = 0x07FFFFFF,
    MASK_28 = 0x0FFFFFFF,
    MASK_29 = 0x1FFFFFFF,
    MASK_30 = 0x3FFFFFFF,
    MASK_31 = 0x7FFFFFFF,
    MASK_32 = 0xFFFFFFFF,
};

}


namespace rcsc {

const double AudioCodec::ERROR_VALUE = std::numeric_limits< double >::max();

/*-------------------------------------------------------------------*/
/*!

*/
AudioCodec::AudioCodec()
{
    createMap( 0 );
}

/*-------------------------------------------------------------------*/
/*!

*/
AudioCodec &
AudioCodec::instance()
{
    static AudioCodec s_instance;

    return s_instance;
}

/*-------------------------------------------------------------------*/
/*!

*/
const
AudioCodec &
AudioCodec::i()
{
    return instance();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AudioCodec::createMap( const int shift )
{
    //[-0-9a-zA-Z ().+*/?<>_]
    M_char_set =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        " ().+-*/?<>_"
        "0123456789";

    const int char_size = static_cast< int >( M_char_set.size() );
    M_int_to_char_map.resize( char_size, '0' );

    int shift_val = shift;
    if ( shift_val < 0 ) shift_val = -shift_val;

    for ( int i = 0; i < char_size; ++i )
    {
        int ch_i = ( i + shift_val ) % char_size;
        char ch = M_char_set[ch_i];

        //M_char_to_int_map.insert( std::make_pair( M_char_set[ch_i], i ) );
        M_char_to_int_map[ ch ] = i;
        M_int_to_char_map[i] = ch;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
AudioCodec::encodeInt64ToStr( const std::int64_t & ival,
                              const int len,
                              std::string & to ) const
{
    std::vector< int > remainder_values;
    remainder_values.reserve( len );

    std::int64_t divided = ival;

    const int char_size = static_cast< int >( M_char_set.size() );

    for ( int i = 0; i < len - 1; ++i )
    {
        remainder_values.push_back( divided % char_size );
        divided /= char_size;
    }

    if ( divided >= char_size )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ***ERROR*** AudioCodec::encodeInt64ToStr."
                  << " Illegal value. "
                  << std::endl;
        return false;
    }

    remainder_values.push_back( divided );

    try
    {
        for ( std::vector< int >::reverse_iterator it = remainder_values.rbegin(), end = remainder_values.rend();
              it != end;
              ++it )
        {
            to += M_int_to_char_map.at( *it );
        }
    }
    catch ( std::exception & e )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ***ERROR*** AudioCodec::encodeInt64ToStr."
                  << " Exception caught: " << e.what()
                  << std::endl;
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
AudioCodec::decodeStrToInt64( const std::string & from,
                              std::int64_t * to ) const
{
    if ( from.empty() )
    {
        return false;
    }

    std::int64_t rval = 0;
    int digit_count = from.length() - 1;

    const int char_size = static_cast< int >( M_char_set.length() );

    for ( std::string::const_iterator ch = from.begin(), end = from.end();
          ch != end;
          ++ch, --digit_count )
    {
        CharToIntCont::const_iterator it = M_char_to_int_map.find( *ch );
        if ( it == M_char_to_int_map.end() )
        {
            std::cerr << __FILE__ << ": " << __LINE__
                      << " ***ERROR*** AudioCodec::decodeStrToInt64."
                      << " Unexpected communication message. ["
                      << from << "]"
                      << std::endl;
            return false;
        }

        rval
            += static_cast< std::int64_t >( it->second )
            * static_cast< std::int64_t >
            ( std::pow( static_cast< double >( char_size ), digit_count ) );
    }

    if ( to )
    {
        *to = rval;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
char
AudioCodec::encodePercentageToChar( const double & value ) const
{
    if ( value < -0.000001 || 1.000001 < value )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ***ERROR*** illegal value range = "
                  << value << std::endl;
        return '\0';
    }

    const int char_size = static_cast< int >( M_char_set.length() );

    int ival = static_cast< int >( rint( value * ( char_size - 1 ) ) * 10000.0 );
    ival /= 10000;
    if ( ival >= char_size )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ***ERROR*** generated illegal index = "
                  << ival << " <- " << value << std::endl;
        return '\0';
    }

    try
    {
        return  intToCharMap().at( ival );
    }
    catch ( ... )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " ***ERROR*** AudioCodec::encodePercentageToChar."
                  << " Exception caught! Failed to encode"
                  << std::endl;
        return '\0';
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
double
AudioCodec::decodeCharToPercentage( const char ch ) const
{
    CharToIntCont::const_iterator it = charToIntMap().find( ch );
    if ( it == charToIntMap().end() )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " ***ERROR*** AudioCodec::decodeCharToPercentage."
                  << " Unsupported character [" << ch << "]"
                  << std::endl;
        return ERROR_VALUE;
    }

    const int char_size = static_cast< int >( M_char_set.length() );

    return ( static_cast< double >( it->second )
             / static_cast< double >( char_size - 1 ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
std::int32_t
AudioCodec::posToBit18( const Vector2D & pos ) const
{
    std::int32_t rval = 0;

    // pos.x value -> 9 bits (=[0,511])
    {
        double x = min_max( -52.0, pos.x, 52.0 );
        x += 52.0;
        x *= ( 511.0 / 104.0 ); // x /= (104.0/511.0);
        rval |= static_cast< std::int32_t >( rint( x ) );
    }

    rval <<= 9; // 9 bits shift for next info

    // pos.y value -> 9 bits (=[0,511])
    {
        double y = min_max( -34.0, pos.y, 34.0 );
        y += 34.0;
        y *= ( 511.0 / 68.0 ); // y /= (68.0/511.0);
        rval |= static_cast< std::int32_t >( rint( y ) );
    }

    return rval;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AudioCodec::bit18ToPos( const std::int32_t & val,
                        Vector2D * pos ) const
{
    // pos.x
    {
        std::int32_t ix = ( val >> 9 ) & MASK_9;
        pos->x = ix * ( 104.0 / 511.0 ) - 52.0;
    }

    // pos.y
    {
        std::int32_t iy = val & MASK_9;
        pos->y = iy * ( 68.0 / 511.0 ) - 34.0;
    }
}


/*-------------------------------------------------------------------*/
/*!

*/
std::int32_t
AudioCodec::posToBit19( const Vector2D & pos ) const
{
    std::int32_t rval = 0;

    // pos.x value -> 10 bits (=[0,1023])
    {
        double x = min_max( -52.5, pos.x, 52.5 );
        x += 52.5;
        x *= ( 1023.0 / 105.0 ); // x /= (105.0/1023.0);
        rval |= static_cast< std::int32_t >( rint( x ) );
    }

    rval <<= 9; // 9 bits shift for next info

    // pos.y value -> 9 bits (=[0,511])
    {
        double y = min_max( -34.0, pos.y, 34.0 );
        y += 34.0;
        y *= ( 511.0 / 68.0 ); // y /= (68.0/511.0);
        rval |= static_cast< std::int32_t >( rint( y ) );
    }

    return rval;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AudioCodec::bit19ToPos( const std::int32_t & val,
                        Vector2D * pos ) const
{
    // pos.x
    {
        std::int32_t ix = ( val >> 9 ) & MASK_10;
        pos->x = ix * ( 105.0 / 1023.0 ) - 52.5;
    }

    // pos.y
    {
        std::int32_t iy = val & MASK_9;
        pos->y = iy * ( 68.0 / 511.0 ) - 34.0;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
std::int32_t
AudioCodec::posVelToBit31( const Vector2D & pos,
                           const Vector2D & vel ) const
{
    const double max_speed = ServerParam::i().ballSpeedMax();

    std::int32_t rval = posToBit19( pos );

#if 1
    {
        rval <<= 1;
        if ( vel.x < 0.0 ) rval |= 1;
        rval <<= 5;
        double vx = std::min( vel.absX(), max_speed );
        vx *= ( 31.0 / max_speed );
        rval |= static_cast< std::int32_t >( rint( vx ) );
    }
    {
        rval <<= 1;
        if ( vel.y < 0.0 ) rval |= 1;
        rval <<= 5;
        double vy = std::min( vel.absY(), max_speed );
        vy *= ( 31.0 / max_speed );
        rval |= static_cast< std::int32_t >( rint( vy ) );
    }
#else
    rval <<= 6; // 6 bits shift for next info

    // vel.x value -> 6 bits (=[0,63])
    {
        double vx = min_max( -max_speed, vel.x, max_speed );
        vx += max_speed;
        vx *= ( 63.0 / ( max_speed*2.0 ) );
        rval |= static_cast< std::int32_t >( rint( vx ) );
    }

    rval <<= 6; // 6 bits shift for next info

    // vel.y value -> 6 bits (=[0,63])
    {
        double vy = min_max( -max_speed, vel.y, max_speed );
        vy += max_speed;
        vy *= ( 63.0 / ( max_speed*2.0 ) );
        rval |= static_cast< std::int32_t >( rint( vy ) );
    }
#endif

    return rval;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AudioCodec::bit31ToPosVel( const std::int32_t & val,
                           Vector2D * pos,
                           Vector2D * vel ) const
{
    const double max_speed = ServerParam::i().ballSpeedMax();

    bit19ToPos( val >> 12, pos );

#if 1
    {
        std::int32_t ivx = ( val >> 6 ) & MASK_5;
        vel->x = ivx * ( max_speed / 31.0 );
        if ( ( val >> 11 ) & MASK_1 )
        {
            vel->x *= -1.0;
        }
    }
    {
        std::int32_t ivy = val & MASK_5;
        vel->y = ivy * ( max_speed / 31.0 );
        if ( ( val >> 5 ) & MASK_1 )
        {
            vel->y *= -1.0;
        }
    }
#else
    // vel.x
    {
        std::int32_t ivx = ( val >> 6 ) & MASK_6;
        vel->x = ivx * ( ( max_speed*2.0 ) / 63.0 ) - max_speed;
    }

    // vel.y
    {
        std::int32_t ivy = val & MASK_6;
        vel->y = ivy * ( ( max_speed*2.0 ) / 63.0 ) - max_speed;
    }
#endif
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
AudioCodec::encodePosVelToStr5( const Vector2D & pos,
                                const Vector2D & vel,
                                std::string & to ) const
{
    std::int64_t ival = posVelToBit31( pos, vel );

    //std::cout << " ival dec = "<< std::dec << ival << std::endl;
    //std::cout << " ival hex = "<< std::hex << ival << std::endl;

    return encodeInt64ToStr( ival, 5, to );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
AudioCodec::decodeStr5ToPosVel( const std::string & from,
                                Vector2D * pos,
                                Vector2D * vel ) const
{
    if ( from.length() != 5 )
    {
        return false;
    }

    std::int64_t read_val = 0;

    if ( ! decodeStrToInt64( from, &read_val ) )
    {
        return false;
    }

    bit31ToPosVel( static_cast< std::int32_t >( read_val ),
                   pos, vel );

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
AudioCodec::encodePosToStr3( const Vector2D & pos,
                             std::string & to ) const
{
    std::int32_t ival = posToBit18( pos );

    return encodeInt64ToStr( static_cast< std::int64_t >( ival ),
                             3, to );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
AudioCodec::decodeStr3ToPos( const std::string & from,
                             Vector2D * pos ) const
{
    if ( from.length() != 3 )
    {
        return false;
    }

    std::int64_t read_val = 0;

    if ( ! decodeStrToInt64( from, &read_val ) )
    {
        return false;
    }

    if ( pos )
    {
        bit18ToPos( static_cast< std::int32_t >( read_val ), pos );
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
AudioCodec::encodeUnumPosToStr4( const int unum,
                                 const Vector2D & pos,
                                 std::string & to ) const
{
    if ( unum < 1 || 11 < unum )
    {
        return false;
    }

    std::int64_t ival = posToBit19( pos );

    ival <<= 4;
    ival |= static_cast< std::int64_t >( unum ); // 4 bits

    return encodeInt64ToStr( ival, 4, to );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
AudioCodec::decodeStr4ToUnumPos( const std::string & from,
                                 int * unum,
                                 Vector2D * pos ) const
{
    if ( from.length() != 4 )
    {
        return false;
    }

    std::int64_t read_val = 0;

    if ( ! decodeStrToInt64( from, &read_val ) )
    {
        return false;
    }

    std::int32_t read_val32 = static_cast< std::int32_t >( read_val );

    if ( unum )
    {
        *unum = static_cast< int >( read_val32 & MASK_4 );
    }

    if ( pos )
    {
        bit19ToPos( read_val32 >> 4, pos );
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::string
AudioCodec::encodeCoordToStr2( const double & xy,
                               const double & norm_factor ) const
{
    const int char_size = static_cast< int >( M_char_set.length() );

    // normalize value
    double tmp = min_max( -norm_factor, xy , norm_factor );
    tmp += norm_factor;

    // round
    tmp /= COORD_STEP_L2;

    int ival = static_cast< int >( rint( tmp ) );

    int i1 = ival % char_size;
    ival /= char_size;
    int i2 = ival % char_size;
    //std::cout << " posx -> " << ix1 << " " << ix2 << std::endl;

    if ( ival >= char_size )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " ***ERROR*** AudioCodec::encodeCoordToStr2."
                  << " range over. value = " << xy
                  << " norm_factor = " << norm_factor
                  << std::endl;
        return std::string();
    }

    std::string msg;
    msg.reserve( 2 );

    try
    {
        msg += intToCharMap().at( i1 );
        msg += intToCharMap().at( i2 );
    }
    catch ( ... )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " ***ERROR*** AudioCodec::encodeCoordToStr2."
                  << " Exception caught! "
                  << std::endl;
        return std::string();
    }

    return msg;
}

/*-------------------------------------------------------------------*/
/*!

*/
double
AudioCodec::decodeStr2ToCoord( const char ch1,
                               const char ch2,
                               const double & norm_factor ) const
{
    const CharToIntCont::const_iterator end = charToIntMap().end();

    CharToIntCont::const_iterator it = charToIntMap().find( ch1 );
    if ( it == end )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " ***ERROR*** AudioCodec::decodeStr2ToCoord()."
                  << " Unsupported character [" << ch1 << "]"
                  << std::endl;
        return ERROR_VALUE;
    }
    int i1 = it->second;

    it = charToIntMap().find( ch2 );
    if ( it == end )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " ***ERROR*** AudioCodec::decodeStr2ToCoord()."
                  << " Unsupported character [" << ch2 << "]"
                  << std::endl;
        return ERROR_VALUE;
    }
    int i2 = it->second;

    const int char_size = static_cast< int >( M_char_set.length() );

    return
        (
         static_cast< double >( i1 + i2 * char_size ) * COORD_STEP_L2
         - norm_factor
         );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::string
AudioCodec::encodePosToStr4( const Vector2D & pos ) const
{
    std::string pos_str;
    pos_str.reserve( 4 );

    pos_str += encodeCoordToStr2( pos.x, X_NORM_FACTOR );
    pos_str += encodeCoordToStr2( pos.y, Y_NORM_FACTOR );

    if ( pos_str.length() != 4 )
    {
        std::cerr << "AudioCodec::encodePosToStr4(). "
                  << "Failed to encode " << pos
                  << std::endl;
        pos_str.clear();
    }

    return pos_str;
}

/*-------------------------------------------------------------------*/
/*!

 */
Vector2D
AudioCodec::decodeStr4ToPos( const std::string & msg ) const
{
    if ( msg.length() < 4 )
    {
        std::cerr << "AudioCodec::decodeStr4ToPos(). "
                  << "message length is too short " << msg
                  << std::endl;
        return Vector2D::INVALIDATED;
    }

    Vector2D pos;

    pos.x = decodeStr2ToCoord( msg[0], msg[1], X_NORM_FACTOR );
    if ( pos.x == ERROR_VALUE )
    {
        std::cerr << "AudioCodec::decodeStr4ToPos(). "
                  << "Unexpected x value " << msg
                  << std::endl;
        return Vector2D::INVALIDATED;
    }

    pos.y = decodeStr2ToCoord( msg[2], msg[3], Y_NORM_FACTOR );
    if ( pos.y == ERROR_VALUE )
    {
        std::cerr << "AudioCodec::decodeStr4ToPos(). "
                  << "Unexpected x value " << msg
                  << std::endl;
        return Vector2D::INVALIDATED;
    }

    return pos;
}

/*-------------------------------------------------------------------*/
/*!

*/
char
AudioCodec::encodeSpeedToChar( const double & val ) const
{
    // normalize value
    double tmp = min_max( -SPEED_NORM_FACTOR, val , SPEED_NORM_FACTOR );
    tmp += SPEED_NORM_FACTOR;

    // round
    tmp /= SPEED_STEP_L1;

    int ival = static_cast< int >( rint( tmp ) );

    try
    {
        return  intToCharMap().at( ival );
    }
    catch ( ... )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " ***ERROR*** AudioCodec::encodeSpeedL1."
                  << " Exception caught! Failed to encode"
                  << std::endl;
        return '\0';
    }

    return '\0';
}

/*-------------------------------------------------------------------*/
/*!

*/
double
AudioCodec::decodeCharToSpeed( const char ch ) const
{
    CharToIntCont::const_iterator it = charToIntMap().find( ch );
    if ( it == charToIntMap().end() )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " ***ERROR*** AudioCodec::decodeSpeedL1."
                  << " Unsupported character [" << ch << "]"
                  << std::endl;
        return ERROR_VALUE;
    }

    return
        ( static_cast< double >( it->second ) * SPEED_STEP_L1
          - SPEED_NORM_FACTOR
          );
}

/*-------------------------------------------------------------------*/
/*!

*/
char
AudioCodec::unum2hex( const int unum )
{
    switch ( unum ) {
    case 1: return '1'; break;
    case 2: return '2'; break;
    case 3: return '3'; break;
    case 4: return '4'; break;
    case 5: return '5'; break;
    case 6: return '6'; break;
    case 7: return '7'; break;
    case 8: return '8'; break;
    case 9: return '9'; break;
    case 10: return 'A'; break;
    case 11: return 'B'; break;
    default:
        break;
    }

    return '\0';
}

/*-------------------------------------------------------------------*/
/*!

*/
int
AudioCodec::hex2unum( const char hex )
{
    switch ( hex ) {
    case '1': return 1; break;
    case '2': return 2; break;
    case '3': return 3; break;
    case '4': return 4; break;
    case '5': return 5; break;
    case '6': return 6; break;
    case '7': return 7; break;
    case '8': return 8; break;
    case '9': return 9; break;
    case 'A': return 10; break;
    case 'B': return 11; break;
    default:
        break;
    }

    return Unum_Unknown;
}

}
