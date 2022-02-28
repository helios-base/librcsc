// -*-c++-*-

/*!
  \file clang_parser.h
  \brief clang parser class Header File
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

#ifndef RCSC_CLANG_PARSER_H
#define RCSC_CLANG_PARSER_H

#include <rcsc/clang/clang_message.h>

#include <memory>
#include <string>

namespace rcsc {

/*!
  \class CLangParser
  \brief clang message parser
 */
class CLangParser {
private:

    //! pimpl ideom
    class Impl;

    std::unique_ptr< Impl > M_impl; //!< pimpl object pointer

    CLangMessage::ConstPtr M_message; //!< analyzed message object

    // not used
    CLangParser( const CLangParser & ) = delete;
    CLangParser & operator=( const CLangParser & ) = delete;

public:

    /*!
      \brief default constructor. create parser implementation.
     */
    CLangParser();

    /*!
      \brief release internal implementation object
     */
    ~CLangParser();

    /*!
      \brief parser interface
      \param msg target string
      \return parsing result
     */
    bool parse( const std::string & msg );

    /*!
      \brief clear all analyzed result.
     */
    void clear();

    /*
      \brief get analyzed message object.
     */
    const CLangMessage::ConstPtr & message() const
      {
          return M_message;
      }

private:

    /*!
      \brief semantic action for clang string.
      \param first the first point of parsed string
      \param last the last point of parsed string
      \return action result.
     */
    bool handleString( const char * first,
                       const char * last );

    /*!
      \brief semantic action for clang variable.
      \param first the first point of parsed string
      \param last the last point of parsed string
      \return action result.
     */
    bool handleVariable( const char * first,
                         const char * last );

    /*!
      \brief semantic action for uniform number.
      \param unum parsed uniform number.
      \return action result.
     */
    bool handleUnum( const int unum );

    /*!
      \brief semantic action for unum_set
      \return action result.
     */
    bool handleUnumSet();


    /*!
      \brief semantic action for boolean condition.
      \param value parsed value.
      \return action result.
     */
    bool handleConditionBool( bool value );

    /*!
      \brief semantic action for do/dont
      \param on do/dont flag
      \return action result.
     */
    bool handlePositive( bool on );

    /*!
      \brief semantic action for our/opp
      \param our our/opp flag
      \return action result.
     */
    bool handleTeam( bool our );

    /*!
      \brief semantic action for mark action
      \return action result.
     */
    bool handleActMark();

    /*!
      \brief semantic action for hetero type
      \param type player type id
      \return action result.
     */
    bool handleActHeteroTypeId( const int type );

    /*!
      \brief semantic action for htype action
      \return action result.
     */
    bool handleActHeteroType();

    /*!
      \brief semantic action for hold action
      \return action result.
     */
    bool handleActHold();

    /*!
      \brief semantic action for bto action
      \return action result.
     */
    bool handleActBallTo();

    /*!
      \brief semantic action for normal directive.
      \return action result.
     */
    bool handleDirectiveCommon();

    /*!
      \brief semantic action for named directive.
      \return action result.
     */
    bool handleDirectiveNamed();

    /*!
      \brief semantic action for token's ttl value.
      \param ttl time to live value.
      \return action result.
     */
    bool handleTokenTTL( int ttl );

    /*!
      \brief semantic action for rule token.
      \return action result.
     */
    bool handleTokenRule();

    /*!
      \brief semantic action for clear token.
      \return action result.
     */
    bool handleTokenClear();

    /*!
      \brief semantic action for info message.
      \return action result.
     */
    bool handleInfoMessage();
};

}

#endif
