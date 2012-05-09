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

namespace rcsc {

const double AudioCodec::ERROR_VALUE = std::numeric_limits< double >::max();

const double AudioCodec::X_NORM_FACTOR = 57.5;
const double AudioCodec::Y_NORM_FACTOR = 39.0;
const double AudioCodec::SPEED_NORM_FACTOR = 3.0;

const double AudioCodec::COORD_STEP_L2 = 0.1;
const double AudioCodec::SPEED_STEP_L1 = 0.1;


//[-0-9a-zA-Z ().+*/?<>_]
const std::string AudioCodec::CHAR_SET =
"0123456789"
"abcdefghijklmnopqrstuvwxyz"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
" ().+-*/?<>_";

const int AudioCodec::CHAR_SIZE = static_cast< int >( CHAR_SET.length() );


/*-------------------------------------------------------------------*/
/*!

*/
AudioCodec::AudioCodec()
{
    // create int <-> char map

    for ( int i = 0; i < CHAR_SIZE; ++i )
    {
        M_char_to_int_map.insert( std::make_pair( CHAR_SET[i], i ) );
        M_int_to_char_map.push_back( CHAR_SET[i] );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
const
AudioCodec &
AudioCodec::i()
{
    static AudioCodec s_instance;

    return s_instance;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
AudioCodec::encodeInt64ToStr( const boost::int64_t & ival,
                              const int len,
                              std::string & to ) const
{
    std::vector< int > remainder_values;
    remainder_values.reserve( len );

    boost::int64_t divided = ival;

    for ( int i = 0; i < len - 1; ++i )
    {
        remainder_values.push_back( divided % CHAR_SIZE );
        divided /= CHAR_SIZE;
    }

    if ( divided >= CHAR_SIZE )
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
        const std::vector< int >::reverse_iterator end = remainder_values.rend();
        for ( std::vector< int >::reverse_iterator it = remainder_values.rbegin();
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
                              boost::int64_t * to ) const
{
    if ( from.empty() )
    {
        return false;
    }

    boost::int64_t rval = 0;
    int digit_count = from.length() - 1;

    const std::string::const_iterator end = from.end();
    for ( std::string::const_iterator ch = from.begin();
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
            += static_cast< boost::int64_t >( it->second )
            * static_cast< boost::int64_t >
            ( std::pow( static_cast< double >( CHAR_SIZE ), digit_count ) );
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

    int ival = static_cast< int >( rint( value * ( CHAR_SIZE - 1 ) ) * 10000.0 );
    ival /= 10000;
    if ( ival >= CHAR_SIZE )
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

    return ( static_cast< double >( it->second )
             / static_cast< double >( CHAR_SIZE - 1) );
}

/*-------------------------------------------------------------------*/
/*!

*/
boost::int32_t
AudioCodec::posToBit18( const Vector2D & pos ) const
{
    boost::int32_t rval = 0;

    // pos.x value -> 9 bits (=[0,511])
    {
        double x = min_max( -52.0, pos.x, 52.0 );
        x += 52.0;
        x *= ( 511.0 / 104.0 ); // x /= (104.0/511.0);
        rval |= static_cast< boost::int32_t >( rint( x ) );
    }

    rval <<= 9; // 9 bits shift for next info

    // pos.y value -> 9 bits (=[0,511])
    {
        double y = min_max( -34.0, pos.y, 34.0 );
        y += 34.0;
        y *= ( 511.0 / 68.0 ); // y /= (68.0/511.0);
        rval |= static_cast< boost::int32_t >( rint( y ) );
    }

    return rval;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AudioCodec::bit18ToPos( const boost::int32_t & val,
                        Vector2D * pos ) const
{
    // pos.x
    {
        boost::int32_t ix = ( val >> 9 ) & MASK_9;
        pos->x = ix * ( 104.0 / 511.0 ) - 52.0;
    }

    // pos.y
    {
        boost::int32_t iy = val & MASK_9;
        pos->y = iy * ( 68.0 / 511.0 ) - 34.0;
    }
}


/*-------------------------------------------------------------------*/
/*!

*/
boost::int32_t
AudioCodec::posToBit19( const Vector2D & pos ) const
{
    boost::int32_t rval = 0;

    // pos.x value -> 10 bits (=[0,1023])
    {
        double x = min_max( -52.5, pos.x, 52.5 );
        x += 52.5;
        x *= ( 1023.0 / 105.0 ); // x /= (105.0/1023.0);
        rval |= static_cast< boost::int32_t >( rint( x ) );
    }

    rval <<= 9; // 9 bits shift for next info

    // pos.y value -> 9 bits (=[0,511])
    {
        double y = min_max( -34.0, pos.y, 34.0 );
        y += 34.0;
        y *= ( 511.0 / 68.0 ); // y /= (68.0/511.0);
        rval |= static_cast< boost::int32_t >( rint( y ) );
    }

    return rval;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AudioCodec::bit19ToPos( const boost::int32_t & val,
                        Vector2D * pos ) const
{
    // pos.x
    {
        boost::int32_t ix = ( val >> 9 ) & MASK_10;
        pos->x = ix * ( 105.0 / 1023.0 ) - 52.5;
    }

    // pos.y
    {
        boost::int32_t iy = val & MASK_9;
        pos->y = iy * ( 68.0 / 511.0 ) - 34.0;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
boost::int32_t
AudioCodec::posVelToBit31( const Vector2D & pos,
                           const Vector2D & vel ) const
{
    const double max_speed = ServerParam::i().ballSpeedMax();

    boost::int32_t rval = posToBit19( pos );

    rval <<= 6; // 6 bits shift for next info

    // vel.x value -> 6 bits (=[0,63])
    {
        //double vx = min_max( -2.7, vel.x, 2.7 );
        //vx += 2.7;
        //vx *= ( 63.0 / 5.4 ); // vx /= (5.4/63.0);
        double vx = min_max( -max_speed, vel.x, max_speed );
        vx += max_speed;
        vx *= ( 63.0 / ( max_speed*2.0 ) );
        rval |= static_cast< boost::int32_t >( rint( vx ) );
    }

    rval <<= 6; // 6 bits shift for next info

    // vel.y value -> 6 bits (=[0,63])
    {
        //double vy = min_max( -2.7, vel.y, 2.7 );
        //vy += 2.7;
        //vy *= ( 63.0 / 5.4 ); // vy /= (5.4/63.0);
        double vy = min_max( -max_speed, vel.y, max_speed );
        vy += max_speed;
        vy *= ( 63.0 / ( max_speed*2.0 ) );
        rval |= static_cast< boost::int32_t >( rint( vy ) );
    }

    return rval;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AudioCodec::bit31ToPosVel( const boost::int32_t & val,
                           Vector2D * pos,
                           Vector2D * vel ) const
{
    const double max_speed = ServerParam::i().ballSpeedMax();

    bit19ToPos( val >> 12, pos );

    // vel.x
    {
        boost::int32_t ivx = ( val >> 6 ) & MASK_6;
        vel->x = ivx * ( ( max_speed*2.0 ) / 63.0 ) - max_speed;
    }

    // vel.y
    {
        boost::int32_t ivy = val & MASK_6;
        vel->y = ivy * ( ( max_speed*2.0 ) / 63.0 ) - max_speed;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
AudioCodec::encodePosVelToStr5( const Vector2D & pos,
                                const Vector2D & vel,
                                std::string & to ) const
{
    boost::int64_t ival = posVelToBit31( pos, vel );

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

    boost::int64_t read_val = 0;

    if ( ! decodeStrToInt64( from, &read_val ) )
    {
        return false;
    }

    bit31ToPosVel( static_cast< boost::int32_t >( read_val ),
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
    boost::int32_t ival = posToBit18( pos );

    return encodeInt64ToStr( static_cast< boost::int64_t >( ival ),
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

    boost::int64_t read_val = 0;

    if ( ! decodeStrToInt64( from, &read_val ) )
    {
        return false;
    }

    if ( pos )
    {
        bit18ToPos( static_cast< boost::int32_t >( read_val ), pos );
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

    boost::int64_t ival = posToBit19( pos );

    ival <<= 4;
    ival |= static_cast< boost::int64_t >( unum ); // 4 bits

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

    boost::int64_t read_val = 0;

    if ( ! decodeStrToInt64( from, &read_val ) )
    {
        return false;
    }

    boost::int32_t read_val32 = static_cast< boost::int32_t >( read_val );

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
    // normalize value
    double tmp = min_max( -norm_factor, xy , norm_factor );
    tmp += norm_factor;

    // round
    tmp /= COORD_STEP_L2;

    int ival = static_cast< int >( rint( tmp ) );

    int i1 = ival % CHAR_SIZE;
    ival /= CHAR_SIZE;
    int i2 = ival % CHAR_SIZE;
    //std::cout << " posx -> " << ix1 << " " << ix2 << std::endl;

    if ( ival >= CHAR_SIZE )
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

    return
        (
         static_cast< double >( i1 + i2 * CHAR_SIZE ) * COORD_STEP_L2
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
