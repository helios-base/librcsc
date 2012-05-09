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

#include <boost/cstdint.hpp>

#include <map>
#include <vector>
#include <string>
#include <cmath>

namespace rcsc {

/*!
  \class AudioCodec
  \brief communication message encoder/decorder
*/
class AudioCodec {
public:
    typedef std::map< char, int > CharToIntCont; //!< map from char to int
    typedef std::vector< char > IntToCharCont; //!< map from int to char

    //! constant error value (= std::numeric_limits< double >::max())
    static const double ERROR_VALUE;

    //! x normalize factor (field length) to limit inputed x
    static const double X_NORM_FACTOR;
    //! y normalze factor (field width) to limit inputed y
    static const double Y_NORM_FACTOR;
    //! speed normalize factor to limit inputed speed range
    static const double SPEED_NORM_FACTOR;

    //! used by encodeCoordToStr2/decodeStr2ToCoord
    static const double COORD_STEP_L2;
    //! used by encodeSpeedToChar/decodeCharToSpeed
    static const double SPEED_STEP_L1;

    /*!
      \enum BitMask
      \brief bit mask enumeration for convenience
     */
    enum BitMask {
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

private:

    //! map to cnvert character to integer. key: char, value int
    CharToIntCont M_char_to_int_map;

    //! map to cnvert integer to character. vector of char
    IntToCharCont M_int_to_char_map;

public:

    static const std::string CHAR_SET; //!< available character set
    static const int CHAR_SIZE; //!< size of CHAR_SET

private:
    /*!
      \brief private for singleton. create convert map.
    */
    AudioCodec();

public:

    /*!
      \brief singleton interface
      \return const reference to the singleton instance
     */
    static
    const
    AudioCodec & i();

private:

    /*!
      \brief encode position to 18 bits info (x:9bits y:9bits)
      \param pos position to be converted
      \return converted integer
    */
    boost::int32_t posToBit18( const Vector2D & pos ) const;

    /*!
      \brief decode 18 bits info to position and velocity
      \param val 32bits integer value to be analyzed
      \param pos variable pointer to store the converted position
    */
    void bit18ToPos( const boost::int32_t & val,
                     Vector2D * pos ) const;

    /*!
      \brief encode position to 19 bits info
      \param pos position to be converted
      \return converted integer
    */
    boost::int32_t posToBit19( const Vector2D & pos ) const;

    /*!
      \brief decode 19 bits info to position and velocity
      \param val 32bits integer value to be analyzed
      \param pos variable pointer to store the converted position
    */
    void bit19ToPos( const boost::int32_t & val,
                     Vector2D * pos ) const;

    /*!
      \brief encode position and velocity to 31 bits info
      \param pos position to be converted
      \param vel velocity to be converted
      \return converted integer
    */
    boost::int32_t posVelToBit31( const Vector2D & pos,
                                  const Vector2D & vel ) const;

    /*!
      \brief decode 31 bits info to position and velocity
      \param val 32bits integer value to be analyzed
      \param pos variable pointer to store the converted position
      \param vel variable pointer to store the converted velocity
    */
    void bit31ToPosVel( const boost::int32_t & val,
                        Vector2D * pos,
                        Vector2D * vel ) const;

public:

    /*!
      \brief get character to interger map object
      \return const reference to the map object
    */
    const
    CharToIntCont & charToIntMap() const
      {
          return M_char_to_int_map;
      }

    /*!
      \brief get integer to character map object
      \return const reference to the map object
    */
    const
    IntToCharCont & intToCharMap() const
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
    bool encodeInt64ToStr( const boost::int64_t & ival,
                           const int len,
                           std::string & to ) const;

    /*!
      \brief decode the message string to the decimal (64bit) integer
      \param from input message string
      \param to pointer to the result instance
      \return decode status
     */
    bool decodeStrToInt64( const std::string & from,
                           boost::int64_t * to ) const;


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
