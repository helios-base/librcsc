// -*-c++-*-

/*!
  \file audio_codec.h
  \brief audio message encoder/decoder Header File
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

#ifndef RCSC_PLAYER_AUDIO_CODEC_H
#define RCSC_PLAYER_AUDIO_CODEC_H

#include <rcsc/geom/vector_2d.h>

#include <unordered_map>
#include <vector>
#include <string>
#include <cmath>
#include <cstdint>

namespace rcsc {

/*!
  \class AudioCodec
  \brief communication message encoder/decorder
*/
class AudioCodec {
public:
    typedef std::unordered_map< char, int > CharToIntCont; //!< map from char to int
    typedef std::vector< char > IntToCharCont; //!< map from int to char

    //! constant error value (= std::numeric_limits< double >::max())
    static const double ERROR_VALUE;

private:

    std::string M_char_set;

    //! map to cnvert character to integer. key: char, value int
    CharToIntCont M_char_to_int_map;

    //! map to cnvert integer to character. vector of char
    IntToCharCont M_int_to_char_map;

    /*!
      \brief private for singleton. create convert map.
    */
    AudioCodec();

public:

    /*!
      \brief singleton interface
      \return reference to the singleton instance
     */
    static AudioCodec & instance();

    /*!
      \brief singleton interface
      \return const reference to the singleton instance
     */
    static const AudioCodec & i();

    void createMap( const int shift );

private:

    /*!
      \brief encode position to 18 bits info (x:9bits y:9bits)
      \param pos position to be converted
      \return converted integer
    */
    std::int32_t posToBit18( const Vector2D & pos ) const;

    /*!
      \brief decode 18 bits info to position and velocity
      \param val 32bits integer value to be analyzed
      \param pos variable pointer to store the converted position
    */
    void bit18ToPos( const std::int32_t & val,
                     Vector2D * pos ) const;

    /*!
      \brief encode position to 19 bits info
      \param pos position to be converted
      \return converted integer
    */
    std::int32_t posToBit19( const Vector2D & pos ) const;

    /*!
      \brief decode 19 bits info to position and velocity
      \param val 32bits integer value to be analyzed
      \param pos variable pointer to store the converted position
    */
    void bit19ToPos( const std::int32_t & val,
                     Vector2D * pos ) const;

    /*!
      \brief encode position and velocity to 31 bits info
      \param pos position to be converted
      \param vel velocity to be converted
      \return converted integer
    */
    std::int32_t posVelToBit31( const Vector2D & pos,
                                  const Vector2D & vel ) const;

    /*!
      \brief decode 31 bits info to position and velocity
      \param val 32bits integer value to be analyzed
      \param pos variable pointer to store the converted position
      \param vel variable pointer to store the converted velocity
    */
    void bit31ToPosVel( const std::int32_t & val,
                        Vector2D * pos,
                        Vector2D * vel ) const;

public:

    /*!
      \brief get character to interger map object
      \return const reference to the map object
    */
    const CharToIntCont & charToIntMap() const
      {
          return M_char_to_int_map;
      }

    /*!
      \brief get integer to character map object
      \return const reference to the map object
    */
    const IntToCharCont & intToCharMap() const
      {
          return M_int_to_char_map;
      }

    /*!
      \brief encode decimal (64bit) integer to the encoded string.
      \param ival input value
      \param len desired string length
      \param to reference to the result instance
      \return encode status
     */
    bool encodeInt64ToStr( const std::int64_t & ival,
                           const int len,
                           std::string & to ) const;

    /*!
      \brief decode the message string to the decimal (64bit) integer
      \param from input message string
      \param to pointer to the result instance
      \return decode status
     */
    bool decodeStrToInt64( const std::string & from,
                           std::int64_t * to ) const;


    /*!
      \brief encode % value to 1 character
      \param value real number [0,1]
      \return encoded character. if value range is illegal, '0' is
      returned.
     */
    char encodePercentageToChar( const double & value ) const;

    /*!
      \brief decode 1 character to the real number [0,1]
      \param ch character to be decoded
      \return decoded value [0,1]. if ch is illegal, negative value
      is returned.
     */
    double decodeCharToPercentage( const char ch ) const;

    /*!
      \brief encode position and velocity to 5 characters.
      \param pos position value to be encoded
      \param vel velocity value to be encoded
      \param to reference to the result variable
      \return encode status

      The length of result string must be 5.
    */
    bool encodePosVelToStr5( const Vector2D & pos,
                             const Vector2D & vel,
                             std::string & to ) const;

    /*!
      \brief decode 5 characters to position and velocity
      \param from string to be decoded
      \param pos variable pointer to store the decoded position value
      \param vel variable pointer to store the decoded velocity value
      \return true if successfully decoded

      The length of 'from' must be 5.
    */
    bool decodeStr5ToPosVel( const std::string & from,
                             Vector2D * pos,
                             Vector2D * vel ) const;

    /*!
      \brief encode position to 3 characters.
      \param pos position value to be encoded
      \param to reference to the result variable
      \return encode status

      The length of result string must be 3.
    */
    bool encodePosToStr3( const Vector2D & pos,
                          std::string & to ) const;

    /*!
      \brief decode 3 characters to and position
      \param from string to be decoded
      \param pos pointer to the result variable
      \return true if successfully decoded

      The length of 'from' must be 3.
    */
    bool decodeStr3ToPos( const std::string & from,
                          Vector2D * pos ) const;


    /*!
      \brief encode uniform number and position to 4 characters.
      \param unum uniform number
      \param pos position value to be encoded
      \param to reference to the result variable
      \return encode status

      The length of result string must be 4.
    */
    bool encodeUnumPosToStr4( const int unum,
                              const Vector2D & pos,
                              std::string & to ) const;

    /*!
      \brief decode 4 characters to uniform number and position
      \param from string to be decoded
      \param unum pointer to the result variable
      \param pos pointer to the result variable
      \return true if successfully decoded

      The length of 'from' must be 4.
    */
    bool decodeStr4ToUnumPos( const std::string & from,
                              int * unum,
                              Vector2D * pos ) const;

    /*!
      \brief encode coordinate value( x or y ) to 2 characters.
      \param xy coordinate value to be encoded, X or Y.
      This value should be within [-norm_factor, norm_factor]
      \param norm_factor normalize factor for xy
      \return result string, or empty string if failed

      Following condition must be satisfied:
      - (norm_factor*2)/COORD_STEP_L2 < CHARSIZE^2

      norm_factor must be same as the value used by decodeCoordL2()
      norm_factor will be changed depending on the type of target
      coordinate value, i.e. field length or field width.
    */
    std::string encodeCoordToStr2( const double & xy,
                                   const double & norm_factor ) const;

    /*!
      \brief decode 2 characters to coordinate value( x or y )
      \param ch1 first character to be decoded
      \param ch2 second character to be decoded
      \param norm_factor normalize factor for coordinate value

      norm_factor must be same as the value used by encodeCoordL2()
      norm_factor will be changed depending on the type of target
      coordinate value, i.e. field length or field width.
    */
    double decodeStr2ToCoord( const char ch1,
                              const char ch2,
                              const double & norm_factor ) const;

    /*!
      \brief encode position value to 4 characters with 0.1 step
      \param pos position value to be encoded
      \return encoded string instance. if encoding is failed, empty string
      is returned
     */
    std::string encodePosToStr4( const Vector2D & pos ) const;

    /*!
      \brief decode 4 characters to position value
      \param from message string to be decoded
      \return decoded position value
     */
    Vector2D decodeStr4ToPos( const std::string & from ) const;

    /*!
      \brief encode speed value to 1 character.
      \param xy velocity element to be encoded, X or Y
      \return result char

      speed value is normalized by SPEED_NORM_FACTOR
    */
    char encodeSpeedToChar( const double & xy ) const;

    /*!
      \brief decode 1 character to speed value
      \param ch character to be decoded
      \return decoded speed value. if failed to decode, HUGE_VAL is returned
    */
    double decodeCharToSpeed( const char ch ) const;

    /*!
      \brief convert decimal uniform number to hex number
      \param unum decimal uniform number, that must be within [1, 11]
      \return hex number character. if invalid number is given, '0' is returned.
     */
    static
    char unum2hex( const int unum );


    /*!
      \brief convert hex number character to decimal uniform number.
      \param hex hex number character, that must be within [0, B], case sensitive!
      \return decimal uniform number, if invalid character is given, 0 is retuned.
     */
    static
    int hex2unum( const char hex );
};

}

#endif
